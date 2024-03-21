#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include "grid.h"

namespace enzo_hll {

//Struct to hold simulation parameters read from file
struct Params {
    int nx;
    double x0;
    double x1;
    double t_final;
    double cfl;
    double gamma;
    std::string output_dir;
    double output_dt;       //Time between output snapshots
    
    //Initial Conditions for a shock tube
    double left_rho, left_u, left_p;
    double right_rho, right_u, right_p;
    double interface_position;
    
    std::string bc_type;
};

//I/O and String Helpers
namespace Utils {

    /**
     *Parses the parameter file.
     */
    Params parseParameterFile(const std::string &path);

    /**
     *Writes a CSV snapshot of the current grid state.
     *Columns: x, rho, u, p, energy
     */
    void writeSnapshotCSV(const std::string &filename, const Grid &g, double time);

    /**
     *Formats an integer step into a zero-padded string (similar to how ENZO does :D, "snapshot_00001.csv")
     */
    std::string formatSnapshotName(int step, int width = 5);

    /**
     *Simple logging helper with timestamp.
     */
    void log(const std::string &message);

} // namespace Utils

} // namespace enzo_hll

#endif // UTILS_H