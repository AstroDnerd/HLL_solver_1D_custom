#include "solver.h"
#include "utils.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>

namespace enzo_hll {

//==========================================
// Physics Conversions
//==========================================

double Solver::soundSpeed(const Primitive &p, double gamma) {
    //c_s = sqrt(gamma * P / rho)
    double press = std::max(p.p, 1e-14);
    double dens = std::max(p.rho, 1e-14);
    return std::sqrt(gamma * press / dens);
}

Primitive Solver::conservedToPrimitive(const Cell &c, double gamma) {
    Primitive p;
    p.rho = std::max(c.rho, 1e-14);
    p.u = c.mom / p.rho;
    
    double kinetic = 0.5 * p.rho * p.u * p.u;
    double internal_eng = c.energy - kinetic;
    
    p.p = (gamma - 1.0) * internal_eng;
    p.p = std::max(p.p, 1e-14);
    
    return p;
}

Cell Solver::primitiveToConserved(const Primitive &p, double gamma) {
    Cell c;
    c.rho = p.rho;
    c.mom = p.rho * p.u;
    
    double internal = p.p / (gamma - 1.0);
    double kinetic = 0.5 * p.rho * p.u * p.u;
    c.energy = internal + kinetic;
    return c;
}

//==========================================
// HLL Flux Implementation
//==========================================

Flux Solver::computeHLLFlux(const Primitive &L, const Primitive &R, double gamma) {
    //STEP 1: Compute derived quantities
    double aL = soundSpeed(L, gamma);
    double aR = soundSpeed(R, gamma);

    //STEP 2: Wave Speed Estimates (Davis approximation)
    double SL = std::min(L.u - aL, R.u - aR);
    double SR = std::max(L.u + aL, R.u + aR);

    //STEP 3: Compute Physical Fluxes F(U) for Left and Right states
    
    // Left Flux
    Cell UL = primitiveToConserved(L, gamma);
    Flux FL;
    FL.rho = UL.mom; // rho * u
    FL.mom = (UL.mom * L.u) + L.p;
    FL.eng = L.u * (UL.energy + L.p);

    // Right Flux
    Cell UR = primitiveToConserved(R, gamma);
    Flux FR;
    FR.rho = UR.mom;
    FR.mom = (UR.mom * R.u) + R.p;
    FR.eng = R.u * (UR.energy + R.p);

    //STEP 4: HLL Logic based on wave speeds relative to interface (x/t = 0)
    Flux hll_flux;

    if (SL >= 0.0) {
        //Supersonic flow to right
        hll_flux = FL;
    } 
    else if (SR <= 0.0) {
        //Supersonic flow to left
        hll_flux = FR;
    } 
    else {
        //Subsonic/Transonic
        double denom = 1.0 / (SR - SL);
        
        hll_flux.rho = (SR * FL.rho - SL * FR.rho + SL * SR * (UR.rho - UL.rho)) * denom;
        hll_flux.mom = (SR * FL.mom - SL * FR.mom + SL * SR * (UR.mom - UL.mom)) * denom;
        hll_flux.eng = (SR * FL.eng - SL * FR.eng + SL * SR * (UR.energy - UL.energy)) * denom;
    }

    return hll_flux;
}

//==========================================
// Time Stepping and Driver
//==========================================

double Solver::computeCFL(const Grid &g, double cfl, double gamma) {
    double max_signal = 0.0;
    
    //Iterate over all cells to find max(|u| + c_s)
    const std::vector<Cell>& cells = g.cells();
    int nx = g.size();

    for(int i = 0; i < nx; ++i) {
        Primitive p = conservedToPrimitive(cells[i], gamma);
        double a = soundSpeed(p, gamma);
        double signal = std::abs(p.u) + a;
        if (signal > max_signal) max_signal = signal;
    }

    if (max_signal < 1e-9) max_signal = 1e-9;

    //dt = CFL * dx / max_signal
    return cfl * g.dx() / max_signal;
}

void Solver::step(Grid &g, double &time, double dt, double gamma, const std::string &bc) {
    int nx = g.size();
    double dx = g.dx();

    //Apply Boundary Conditions
    g.applyBoundaryConditions(bc);

    //Compute Fluxes at Interfaces
    std::vector<Flux> fluxes(nx + 1);
    const std::vector<Cell>& cells = g.cells();

    //Loop over internal interfaces 
    for (int i = 0; i <= nx; ++i) {
        Primitive L, R;

        if (i == 0) {
            //Left boundary interface
            R = conservedToPrimitive(cells[0], gamma);
            L = R;
            if (bc == "reflective") L.u = -R.u; 
        } 
        else if (i == nx) {
            // Right boundary interface
            L = conservedToPrimitive(cells[nx - 1], gamma);
            R = L;
            if (bc == "reflective") R.u = -L.u;
        } 
        else {
            // Interior
            L = conservedToPrimitive(cells[i - 1], gamma);
            R = conservedToPrimitive(cells[i], gamma);
        }

        fluxes[i] = computeHLLFlux(L, R, gamma);
    }

    //Update Conserved Variables
    for (int i = 0; i < nx; ++i) {
        Cell c = cells[i]; // copy old state
        
        double ratio = dt / dx;
        
        c.rho    -= ratio * (fluxes[i+1].rho - fluxes[i].rho);
        c.mom    -= ratio * (fluxes[i+1].mom - fluxes[i].mom);
        c.energy -= ratio * (fluxes[i+1].eng - fluxes[i].eng);

        g.setCell(i, c);
    }

    //Update Time
    time += dt;
}

void Solver::runSimulation(Grid &g, const Params &p) {
    double time = 0.0;
    int step_count = 0;
    int snapshot_idx = 0;
    double time_since_last_output = 0.0;

    Utils::log("Starting Simulation...");
    Utils::log("Output directory: " + p.output_dir);

    //Initial output
    std::string fname = p.output_dir + "/" + Utils::formatSnapshotName(snapshot_idx++);
    Utils::writeSnapshotCSV(fname, g, time);

    while (time < p.t_final) {
        double dt = computeCFL(g, p.cfl, p.gamma);

        //Adjust dt
        if (time + dt > p.t_final) {
            dt = p.t_final - time;
        }

        //Evolve
        step(g, time, dt, p.gamma, p.bc_type);
        step_count++;


        time_since_last_output += dt;

        if (time_since_last_output >= p.output_dt) {
            fname = p.output_dir + "/" + Utils::formatSnapshotName(snapshot_idx++);
            Utils::writeSnapshotCSV(fname, g, time);
            
            time_since_last_output = 0.0;
            
            std::cout << "[Step " << step_count << "] Time: " << time 
                      << ", dt: " << dt << std::endl;
        }
    }

    fname = p.output_dir + "/" + Utils::formatSnapshotName(snapshot_idx);
    Utils::writeSnapshotCSV(fname, g, time);

    Utils::log("Simulation Complete.");
}

} // namespace enzo_hll