#ifndef SOLVER_H
#define SOLVER_H

#include "grid.h"
#include "utils.h"
#include <vector>
#include <string>

namespace enzo_hll {

//Structure to hold Flux vectors
struct Flux {
    double rho;
    double mom;
    double eng;
};

class Solver {
public:
    //Physics Helpers
    static double soundSpeed(const Primitive &p, double gamma);
    static Primitive conservedToPrimitive(const Cell &c, double gamma);
    static Cell primitiveToConserved(const Primitive &p, double gamma);

    //HLL
    /**
     *Computes the HLL approximate Riemann flux at an interface.
     *Params:
     *left : Reconstructed state to the left of interface
     *right : Reconstructed state to the right of interface
     */
    static Flux computeHLLFlux(const Primitive &left, const Primitive &right, double gamma);

    /**
     *Computes stable time step based on CFL condition.
     * dt = CFL * dx / max(|u| + a)
     */
    static double computeCFL(const Grid &g, double cfl, double gamma);

    /**
     *Evolves the Grid by one time step dt.
     * 1. Reconstruct boundary states
     * 2. Compute Fluxes
     * 3. Update Conserved Variables
     */
    static void step(Grid &g, double &time, double dt, double gamma, const std::string &bc);

    // Main Driver
    /**
     *Runs the simulation loop until t_final.
     *Handles I/O scheduling via the Utils helpers.
     */
    static void runSimulation(Grid &g, const Params &p);
};

} // namespace enzo_hll

#endif // SOLVER_H