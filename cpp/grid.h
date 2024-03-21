#ifndef GRID_H
#define GRID_H

#include <vector>
#include <string>
#include <cmath>

namespace enzo_hll {

//Global constant for Adiabatic Index
constexpr double GAMMA = 1.4;

/**
 *Represents the conserved variables at a single grid point.
 */
struct Cell {
    double rho;    // Density
    double mom;    // Momentum (rho * u)
    double energy; // Total Energy per unit volume (E)
};

/**
 *Represents the primitive variables.
 */
struct Primitive {
    double rho; // Density
    double u;   // Velocity
    double p;   // Pressure
};

class Grid {
public:
    /**
     *Construct a new Grid object
     *Parameters:
     *nx : Number of spatial cells
     *x0 : Domain start
     *x1 : Domain end
     */
    Grid(int nx, double x0, double x1);

    /**
     *Initialize the grid with a shock-tube setup, a.k.a The famous Riemann problem.
     *Parameters:
     *left : Primitive state for x < interface_pos
     *right : Primitive state for x >= interface_pos
     *interface_pos : Position of the discontinuity
     */
    void initialize(const Primitive &left, const Primitive &right, double interface_pos);


    int size() const;
    double dx() const;
    double getX0() const;
    const std::vector<Cell>& cells() const;
    
    /**
     *Convert current Conserved state to Primitives.
     *For visualization and boundary conditions.
     */
    std::vector<Primitive> primitives() const;

    void setCell(int i, const Cell &c);
    Cell getCell(int i) const;

    /**
     *Apply boundary conditions to ghost cells or domain edges.
     *Nikhil: In this simple 1D FVM, we modify the edges (0 and N-1) or could be extended to use ghost layers. 
     *Current implementation: Modifies first and last active cells based on type.
     *Parameter :
     *bc_type : "outflow" or "reflective"
     */
    void applyBoundaryConditions(const std::string &bc_type);

    /**
     *Helper to format a specific cell's data for CSV output.
     *Format: x_coord, rho, mom, energy
     */
    std::string getCellCSVString(int i) const;

private:
    int nx_;
    double x0_;
    double x1_;
    double dx_;

    //Primary state storage: Contiguous memory
    std::vector<Cell> cells_;
};

} // namespace enzo_hll

#endif // GRID_H