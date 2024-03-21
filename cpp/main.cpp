#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "grid.h"
#include "solver.h"
#include "utils.h"

namespace fs = std::filesystem;
using namespace enzo_hll;

int main(int argc, char* argv[]) {
    //Argument Validation
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <parameter_file.enzo>" << std::endl;
        return 1;
    }

    std::string param_file = argv[1];
    
    //Parse Parameters
    Utils::log("Reading parameter file: " + param_file);
    Params params = Utils::parseParameterFile(param_file);

    //Setup Output Directory
    try {
        if (!fs::exists(params.output_dir)) {
            fs::create_directories(params.output_dir);
            Utils::log("Created output directory: " + params.output_dir);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating output directory: " << e.what() << std::endl;
        return 1;
    }

    //Initialize Grid by constructing the mesh
    Grid grid(params.nx, params.x0, params.x1);

    Primitive leftState = {params.left_rho, params.left_u, params.left_p};
    Primitive rightState = {params.right_rho, params.right_u, params.right_p};

    //Apply initial conditions
    grid.initialize(leftState, rightState, params.interface_position);

    Utils::log("Grid initialized with " + std::to_string(params.nx) + " cells.");
    Utils::log("Domain: [" + std::to_string(params.x0) + ", " + std::to_string(params.x1) + "]");

    //Run Simulation!! YaY
    try {
        Solver::runSimulation(grid, params);
    } catch (const std::exception& e) {
        std::cerr << "Simulation Runtime Error: " << e.what() << std::endl;
        return 1;
    }

    Utils::log("Success. Exiting.");
    return 0;
}