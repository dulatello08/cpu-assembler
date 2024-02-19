//
// Created by Dulat S on 2/17/24.
//

#include <iostream>
#include "linker.h"
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::vector<std::string> inputFiles;
    std::string configFile;
    std::string outputFile = "a.out";

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_files> -c <config_file> [-o <output_file>]" << std::endl;
        return 1;
    }

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-c") {
            if (i + 1 < argc) {
                configFile = argv[++i];
            } else {
                std::cerr << "Error: Missing configuration file after -c" << std::endl;
                return 1;
            }
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Error: Missing output file name after -o" << std::endl;
                return 1;
            }
        } else {
            inputFiles.push_back(arg);
        }
    }

    if (inputFiles.empty()) {
        std::cerr << "Error: No input files specified" << std::endl;
        return 1;
    }

    if (configFile.empty()) {
        std::cerr << "Error: No configuration file specified" << std::endl;
        return 1;
    }

    // Linking process (placeholder)
    std::cout << "Linking files: ";
    for (const auto& file : inputFiles) {
        std::cout << file << " ";
    }
    std::cout << "\nUsing configuration: " << configFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;

    // Actual linking logic would go here

    return 0;
}
