import os
import sys
import subprocess
import glob
import numpy as np
import pandas as pd
import json
import time
from parser import parse_params

#Config
CPP_DIR = "cpp"
DATA_DIR = "data"
OUTPUTS_DIR = os.path.join(DATA_DIR, "outputs")
BINARY_NAME = "nikhil_hll"

def compile_cpp():
    """
    Compiles the C++ source files into a binary.
    """
    print(f"[run.py] Compiling C++ code in {CPP_DIR}...")
    
    sources = [
        os.path.join(CPP_DIR, "main.cpp"),
        os.path.join(CPP_DIR, "grid.cpp"),
        os.path.join(CPP_DIR, "solver.cpp"),
        os.path.join(CPP_DIR, "utils.cpp")
    ]
    
    cmd = ["g++", "-O3", "-std=c++17"] + sources + ["-o", BINARY_NAME]
    
    try:
        subprocess.check_call(cmd)
        print(f"[run.py] Compilation successful. Binary: ./{BINARY_NAME}")
    except subprocess.CalledProcessError:
        print(f"[run.py] Compilation failed!")
        sys.exit(1)

def run_simulation(param_file):
    """
    Invokes the C++ binary with the given parameter file.
    """
    if not os.path.exists(BINARY_NAME):
        compile_cpp()
        
    print(f"[run.py] Running simulation with {param_file}...")
    
    if os.path.exists(OUTPUTS_DIR):
        for f in glob.glob(os.path.join(OUTPUTS_DIR, "*.csv")):
            os.remove(f)
    else:
        os.makedirs(OUTPUTS_DIR)

    try:
        start_time = time.time()
        # Call C++ binary
        subprocess.check_call([f"./{BINARY_NAME}", param_file])
        elapsed = time.time() - start_time
        print(f"[run.py] Simulation finished in {elapsed:.2f}s.")
    except subprocess.CalledProcessError:
        print("[run.py] Simulation binary crashed.")
        sys.exit(1)

def aggregate_data(params):
    """
    Reads CSV snapshots and stacks them into a single .npy array, also saves a JSON metadata file for visualization context.
    """
    print("[run.py] Aggregating data...")
    
    #Find all snapshot files
    csv_files = sorted(glob.glob(os.path.join(OUTPUTS_DIR, "snapshot_*.csv")))
    if not csv_files:
        print("[run.py] No output files found!")
        return

    #Inspect dimensions
    df_0 = pd.read_csv(csv_files[0])
    nx = len(df_0)
    cols = df_0.columns.tolist() # e.g. ['x', 'rho', 'u', 'p', 'energy']
    n_vars = len(cols)
    n_steps = len(csv_files)
    
    print(f"[run.py] Found {n_steps} snapshots. Grid: {nx} cells. Vars: {cols}")

    sim_data = np.zeros((n_steps, nx, n_vars), dtype=np.float64)
    times = []

    #Load data
    for t_idx, fname in enumerate(csv_files):
        data = np.loadtxt(fname, delimiter=',', skiprows=1)
        sim_data[t_idx, :, :] = data
        times.append(t_idx * params['output_dt'])

    #Save data & metadata
    out_npy = os.path.join(DATA_DIR, "simulation.npy")
    np.save(out_npy, sim_data)
    print(f"[run.py] Saved simulation data to {out_npy}")

    metadata = {
        "params": params,
        "columns": cols,
        "times": times, 
        "shape": list(sim_data.shape)
    }
    
    out_meta = os.path.join(DATA_DIR, "simulation_metadata.json")
    with open(out_meta, 'w') as f:
        json.dump(metadata, f, indent=4)
    print(f"[run.py] Saved metadata to {out_meta}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python run.py <parameter_file.enzo> [--rebuild]")
        sys.exit(1)
        
    param_file = sys.argv[1]
    
    #Check for rebuild
    if "--rebuild" in sys.argv or not os.path.exists(BINARY_NAME):
        compile_cpp()

    #Parse Params, run sim, aggregate data :D
    try:
        params = parse_params(param_file)
    except Exception as e:
        print(f"[run.py] Error parsing parameters: {e}")
        sys.exit(1)

    run_simulation(param_file)

    aggregate_data(params)

if __name__ == "__main__":
    main()