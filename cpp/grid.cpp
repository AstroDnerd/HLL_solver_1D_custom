#include "grid.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace enzo_hll {

//==========================================
//HELPERS
//==========================================
Primitive conservedToPrimitive(const Cell& c) {
    Primitive p;
    //Protect against vacuum/negative density 
    p.rho = c.rho;
    if (p.rho <= 1e-14) p.rho = 1e-14;

    p.u = c.mom / p.rho;
    
    //Internal Energy = Total Energy - Kinetic Energy
    double kinetic = 0.5 * p.rho * p.u * p.u;
    double internal_energy = c.energy - kinetic;
    
    //Ideal Gas Law
    p.p = (GAMMA - 1.0) * internal_energy;
    
    if (p.p <= 1e-14) p.p = 1e-14;

    return p;
}

Cell primitiveToConserved(const Primitive& p) {
    Cell c;
    c.rho = p.rho;
    c.mom = p.rho * p.u;
    
    double internal = p.p / (GAMMA - 1.0);
    double kinetic = 0.5 * p.rho * p.u * p.u;
    c.energy = internal + kinetic;
    
    return c;
}

//==========================================
//GRID IMPLEMENTATION
//==========================================

Grid::Grid(int nx, double x0, double x1) 
    : nx_(nx), x0_(x0), x1_(x1) {
    
    if (nx_ <= 0) nx_ = 100; //Default
    dx_ = (x1_ - x0_) / static_cast<double>(nx_);
    
    //Allocate memory for the solution, IMPORTANT!
    cells_.resize(nx_);
}

void Grid::initialize(const Primitive &left, const Primitive &right, double interface_pos) {
    Cell cLeft = primitiveToConserved(left);
    Cell cRight = primitiveToConserved(right);

    for (int i = 0; i < nx_; ++i) {
        //Cell center coordinate
        double x = x0_ + (i + 0.5) * dx_;
        
        if (x < interface_pos) {
            cells_[i] = cLeft;
        } else {
            cells_[i] = cRight;
        }
    }
}

int Grid::size() const {
    return nx_;
}

double Grid::dx() const {
    return dx_;
}

double Grid::getX0() const {
    return x0_;
}

const std::vector<Cell>& Grid::cells() const {
    return cells_;
}

std::vector<Primitive> Grid::primitives() const {
    std::vector<Primitive> prims;
    prims.reserve(nx_);
    for (const auto& c : cells_) {
        prims.push_back(conservedToPrimitive(c));
    }
    return prims;
}

void Grid::setCell(int i, const Cell &c) {
    if (i >= 0 && i < nx_) {
        cells_[i] = c;
    }
}

Cell Grid::getCell(int i) const {
    if (i >= 0 && i < nx_) {
        return cells_[i];
    }
    //Return empty cell if out of bounds
    return {0.0, 0.0, 0.0};
}

void Grid::applyBoundaryConditions(const std::string &bc_type) {
    if (nx_ < 2) return;

    if (bc_type == "outflow" || bc_type == "transmissive") {
        //Zero-gradient:copy neighbor into edge
    } 
    else if (bc_type == "reflective") {
        //Reflect velocity at boundaries
        cells_[0].mom = -cells_[0].mom; 
        cells_[nx_-1].mom = -cells_[nx_-1].mom;
    }
}

std::string Grid::getCellCSVString(int i) const {
    if (i < 0 || i >= nx_) return "";
    
    double x = x0_ + (i + 0.5) * dx_;
    const Cell& c = cells_[i];
    
    std::stringstream ss;
    ss << std::scientific << std::setprecision(6);
    ss << x << "," << c.rho << "," << c.mom << "," << c.energy;
    return ss.str();
}

} // namespace enzo_hll