# Design Document: HLL Solver & Architecture

## 1. Objective
To implement a transparent and minimal 1D Riemann solver that bridges the gap between low-level numerical implementations (C++) and high-level data analysis workflows (Python). The design mimics the modularity found in larger astrophysical codes like ENZO ([The ENZO Project](https://enzo-project.org/))


## 2. Numerical Method

### Physics Model
We solve the **Euler Equations** for inviscid fluid dynamics in 1D conservation form:
$$\frac{\partial U}{\partial t} + \frac{\partial F(U)}{\partial x} = 0$$

Where:
* **Conserved Variables ($U$):** Density ($\rho$), Momentum ($\rho u$), Total Energy ($E$).
* **Fluxes ($F$):** Mass flux ($\rho u$), Momentum flux ($\rho u^2 + p$), Energy flux ($u(E+p)$).
* **Equation of State:** Ideal Gas Law, closing the system with $p = (\gamma - 1)(E - \frac{1}{2}\rho u^2)$.

### Riemann Solver (HLL)
The inter-cell fluxes are computed using the **HLL (Harten-Lax-van Leer)** approximate Riemann solver.
1.  **Wave Speed Estimation:** We use the Davis estimate for wave speeds $S_L$ and $S_R$:
    $$S_L = \min(u_L - c_L, u_R - c_R)$$
    $$S_R = \max(u_L + c_L, u_R + c_R)$$
2.  **Flux Calculation:**
    * Supersonic Left ($S_L \ge 0$): $F_{HLL} = F_L$
    * Supersonic Right ($S_R \le 0$): $F_{HLL} = F_R$
    * Subsonic (Intermediate): $F_{HLL} = \frac{S_R F_L - S_L F_R + S_L S_R (U_R - U_L)}{S_R - S_L}$

### Time Integration
* **Method:** Explicit Forward Euler (First Order).
* **Stability:** Controlled via the CFL condition. The time step $dt$ is calculated dynamically:
    $$dt = \text{CFL} \times \frac{dx}{\max(|u| + c_s)}$$

## 3. Architecture & Implementation

### Data Structures
* **Grid Layout:** We utilize an **Array of Structures (AoS)** pattern via `std::vector<Cell>`.
    * *Reasoning:* For 1D codes, AoS offers better code readability and easier logical grouping of cell properties compared to SoA. (Note: For high-performance vectorized 3D codes, SoA would be preferred. Also done in ENZO).
* **Ghost Cells:** Boundary conditions are handled by setting the state of ghost cells (implied at indices `0` and `nx`) prior to flux computation, keeping the core loop divergence-free.

### Hybrid Pipeline
1.  **Python (`parser.py`):** Validates inputs and preserves physics metadata (Gamma, CFL) which is often lost in raw binary outputs.
2.  **C++ (`solver.cpp`):** Performs the heavy lifting. Outputs raw CSVs for maximum transparency and easy debugging.
3.  **Python (`run.py`):** Acts as the driver. Compiles code on-the-fly, cleans directories, runs the binary, and efficiently aggregates thousands of CSV rows into a single binary tensor (`simulation.npy`).
