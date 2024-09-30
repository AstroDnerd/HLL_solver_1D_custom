import os
import sys
import json
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

plt.style.use('seaborn-v0_8-darkgrid')

def load_simulation(data_dir="data"):
    """
    Loads simulation data and metadata.
    Returns:
        tuple: (sim_data, metadata) where sim_data is (nt, nx, nvars) and metadata is a dict
    """
    npy_path = os.path.join(data_dir, "simulation.npy")
    json_path = os.path.join(data_dir, "simulation_metadata.json")
    
    if not os.path.exists(npy_path) or not os.path.exists(json_path):
        print(f"Error: Data not found in {data_dir}. Run the simulation first.")
        sys.exit(1)
        
    print(f"Loading data from {npy_path}...")
    sim_data = np.load(npy_path)
    
    with open(json_path, 'r') as f:
        metadata = json.load(f)
        
    return sim_data, metadata

def plot_profiles(sim_data, metadata, output_dir="data/plots"):
    """
    Generates static profile plots for density, velocity, and pressure at selected time snapshots (Start, Mid, End).
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    cols = metadata["columns"]  # e.g., ['x', 'rho', 'u', 'p', 'energy']
    times = metadata["times"]
    
    try:
        idx_x = cols.index('x')
        idx_rho = cols.index('rho')
        idx_u = cols.index('u')
        idx_p = cols.index('p')
    except ValueError as e:
        print(f"Error: Missing expected column in data: {e}")
        return

    n_steps = sim_data.shape[0]
    indices = np.linspace(0, n_steps - 1, 6, dtype=int)
    
    #Create a figure for each variable
    vars_to_plot = [
        ('Density', idx_rho, 'rho'),
        ('Velocity', idx_u, 'u'),
        ('Pressure', idx_p, 'p')
    ]
    
    for label, var_idx, filename_suffix in vars_to_plot:
        plt.figure(figsize=(10, 6))
        
        for i in indices:
            t = times[i]
            x = sim_data[i, :, idx_x]
            y = sim_data[i, :, var_idx]
            plt.plot(x, y, label=f"t = {t:.3f}")
            
        plt.title(f"1D HLL Shock Tube: {label}")
        plt.xlabel("Position (x)")
        plt.ylabel(label)
        plt.legend()
        plt.tight_layout()
        
        save_path = os.path.join(output_dir, f"profile_{filename_suffix}.png")
        plt.savefig(save_path, dpi=150)
        print(f"Saved plot: {save_path}")
        plt.close()

def create_animation(sim_data, metadata, output_file="data/plots/simulation.mp4"):
    """
    Creates an MP4 animation
    """
    if os.system("which ffmpeg > /dev/null 2>&1") != 0:
        print("Warning: ffmpeg not found. Skipping animation.")
        return

    print("Generating animation...")
    cols = metadata["columns"]
    times = metadata["times"]
    
    idx_x = cols.index('x')
    idx_rho = cols.index('rho')
    idx_u = cols.index('u')
    idx_p = cols.index('p')

    fig, axes = plt.subplots(3, 1, figsize=(8, 10), sharex=True)
    
    #Initial data
    x = sim_data[0, :, idx_x]
    
    line_rho, = axes[0].plot(x, sim_data[0, :, idx_rho], 'r-', lw=2)
    axes[0].set_ylabel('Density')
    axes[0].set_title(f"t = {times[0]:.3f}")
    
    line_u, = axes[1].plot(x, sim_data[0, :, idx_u], 'g-', lw=2)
    axes[1].set_ylabel('Velocity')
    
    line_p, = axes[2].plot(x, sim_data[0, :, idx_p], 'b-', lw=2)
    axes[2].set_ylabel('Pressure')
    axes[2].set_xlabel('Position')

    #Set axis limits dynamically
    for ax, idx in zip(axes, [idx_rho, idx_u, idx_p]):
        data_min = np.min(sim_data[:, :, idx])
        data_max = np.max(sim_data[:, :, idx])
        margin = (data_max - data_min) * 0.1
        ax.set_ylim(data_min - margin, data_max + margin)

    def update(frame):
        t = times[frame]
        axes[0].set_title(f"t = {t:.3f}")
        
        line_rho.set_ydata(sim_data[frame, :, idx_rho])
        line_u.set_ydata(sim_data[frame, :, idx_u])
        line_p.set_ydata(sim_data[frame, :, idx_p])
        return line_rho, line_u, line_p

    anim = animation.FuncAnimation(fig, update, frames=len(times), interval=50, blit=False) #Interval is in ms
    
    try:
        anim.save(output_file, writer='ffmpeg', fps=30, dpi=150)
        print(f"Saved animation: {output_file}")
    except Exception as e:
        print(f"Error saving animation: {e}")

    plt.close()

if __name__ == "__main__":
    if not os.path.exists("data"):
        print("Error: data directory missing. Run run.py first.")
        sys.exit(1)

    data, meta = load_simulation()
    
    #Plot Profiles and animate!
    plot_profiles(data, meta)
    
    create_animation(data, meta)