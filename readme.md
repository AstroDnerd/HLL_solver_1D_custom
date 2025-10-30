# ENZO-Inspired 1D HLL Shock Tube Solver

A hybrid C++ / Python simulation pipeline that implements a 1D HLL Approximate Riemann Solver for the Euler equations.

This project demonstrates a high-performance numerical core (C++17) wrapped in a robust automation layer (Python), capable of simulating classic fluid dynamics problems like the **Sod Shock Tube**.

## Features
* **Numerical Core:** C++ implementation of the HLL (Harten-Lax-van Leer) flux scheme.
* **Time Integration:** Explicit time-stepping with dynamic CFL condition.
* **Hybrid Workflow:** Python handles compilation, input parsing, data aggregation, and visualization; C++ handles the heavy compute.
* **Visualization:** Automated generation of profile plots and evolution GIFs.
* **Reproducibility:** Metadata preservation (JSON) alongside raw binary data (NPY).

## Directory Structure
```text
.
├── cpp/                 # C++ Source (Numerical Engine)
│   ├── grid.h/cpp       # Mesh and State management
│   ├── solver.h/cpp     # HLL Flux & Time integration
│   ├── utils.h/cpp      # I/O and Helpers
│   └── main.cpp         # Entry point
├── python/              # Python Tools (Orchestration)
│   ├── parser.py        # Config file parsing
│   ├── run.py           # Build system & Simulation runner
│   └── visualize.py     # Plotting & Animation
├── data/                # Inputs & Outputs
│   ├── shock_tube.enzo  # Simulation parameters
│   ├── outputs/         # Raw CSV snapshots
│   └── plots/           # profile plots and animation
└── README.md
```

## Quick Start

### 1. Prerequisites
* **C++ Compiler:** `g++` (supporting C++17)
* **Python 3.x**
* **Dependencies:** `numpy`, `pandas`, `matplotlib`
    ```bash
    pip install numpy pandas matplotlib
    ```

### 2. Configure & Run
The parameters are defined in `data/shock_tube.enzo` (ENZO-style format).

```bash
#1. Run the simulation (Auto-compiles C++ code)
python3 python/run.py data/shock_tube.enzo --rebuild

#2. Visualize the results (Generates plots and GIF)
python3 python/visualize.py
```

### 3. View Results
Check the `data/plots/` directory:
* `simulation.gif`: Animation of density, velocity, and pressure evolution.
* `profile_*.png`: Static profiles at different time steps for pressure, density and velocity.

## Sample Output
`![Shock Tube GIF](data/plots/simulation.gif)`

## Configuration
The solver is controlled by `.enzo` files. Example:
```ini
nx = 400
x0 = 0.0
x1 = 1.0
interface_position = 0.5
gamma = 1.4
cfl = 0.45
t_final = 0.2
output_dt = 0.01  # Snapshot cadence in simulation time
bc_type = outflow # Options: outflow, reflective
```