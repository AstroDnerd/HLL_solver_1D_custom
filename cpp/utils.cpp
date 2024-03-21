#include "utils.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cmath>
#include <map>

namespace enzo_hll {
namespace Utils {

Params parseParameterFile(const std::string &path) {
    Params p;
    // Set defaults
    p.nx = 100; p.x0 = 0.0; p.x1 = 1.0;
    p.t_final = 0.2; p.cfl = 0.8; p.gamma = 1.4;
    p.output_dt = 0.01;
    p.output_dir = "data/outputs";
    p.bc_type = "outflow";
    p.left_rho = 1.0; p.left_u = 0.0; p.left_p = 1.0;
    p.right_rho = 0.125; p.right_u = 0.0; p.right_p = 0.1;
    p.interface_position = 0.5;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open parameter file " << path 
                  << ". Using defaults." << std::endl;
        return p;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        
        size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos) continue;

        std::string key = line.substr(0, equal_pos);
        std::string val_str = line.substr(equal_pos + 1);

        //Trim whitespace
        auto trim = [](std::string &s) {
            const char* ws = " \t\n\r\f\v";
            s.erase(s.find_last_not_of(ws) + 1);
            s.erase(0, s.find_first_not_of(ws));
        };
        trim(key);
        trim(val_str);

        //Parse Map
        if (key == "nx") p.nx = std::stoi(val_str);
        else if (key == "x0") p.x0 = std::stod(val_str);
        else if (key == "x1") p.x1 = std::stod(val_str);
        else if (key == "t_final") p.t_final = std::stod(val_str);
        else if (key == "cfl") p.cfl = std::stod(val_str);
        else if (key == "gamma") p.gamma = std::stod(val_str);
        else if (key == "output_dt") p.output_dt = std::stod(val_str);
        else if (key == "output_dir") p.output_dir = val_str;
        else if (key == "bc_type") p.bc_type = val_str;
        else if (key == "left_rho") p.left_rho = std::stod(val_str);
        else if (key == "left_u") p.left_u = std::stod(val_str);
        else if (key == "left_p") p.left_p = std::stod(val_str);
        else if (key == "right_rho") p.right_rho = std::stod(val_str);
        else if (key == "right_u") p.right_u = std::stod(val_str);
        else if (key == "right_p") p.right_p = std::stod(val_str);
        else if (key == "interface_position") p.interface_position = std::stod(val_str);
    }
    
    return p;
}

void writeSnapshotCSV(const std::string &filename, const Grid &g, double time) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    outfile << "x,rho,u,p,energy\n";
    
    outfile << std::scientific << std::setprecision(6);

    const auto& cells = g.cells();
    int nx = g.size();
    double dx = g.dx();
    
    std::vector<Primitive> prims = g.primitives();

    // Iterate
    double start_x = g.getX0();
    
    for (int i = 0; i < nx; i++) {
        double current_x = start_x + (i + 0.5) * dx;
        
        const Primitive& p = prims[i];
        const Cell& c = cells[i];

        outfile << current_x << ","
                << p.rho << ","
                << p.u << ","
                << p.p << ","
                << c.energy << "\n";
    }

    outfile.close();
}

std::string formatSnapshotName(int step, int width) {
    std::ostringstream ss;
    ss << "snapshot_" << std::setw(width) << std::setfill('0') << step << ".csv";
    return ss.str();
}

void log(const std::string &message) {
    //stderr logging
    std::cerr << "[ENZO-HLL] " << message << std::endl;
}

} // namespace Utils
} // namespace enzo_hll