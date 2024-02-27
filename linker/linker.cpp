//
// Created by Dulat S on 2/17/24.
//

#include <iostream>
#include "linker.h"

#include <getopt.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "object_files_parser.h"

linker_config parseLinkerConfig(const std::string& config_filename);

int main(const int argc, char* argv[]) {
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
    auto o_files_parser = new object_files_parser(inputFiles);

    o_files_parser->validate_all_files();
    std::vector<uint8_t> mainProgramFile = o_files_parser->readFile("program.o");
    auto [first, second] = o_files_parser->findGlobalStartRange();
    std::cout << first << " " << second << std::endl;
    std::vector<uint8_t> startCode(mainProgramFile.begin() + first, mainProgramFile.begin() + second);

    linker_config linker_config = parseLinkerConfig(configFile);



    delete o_files_parser;
    return 0;
}


linker_config parseLinkerConfig(const std::string& config_filename) {
    linker_config config{};
    std::ifstream config_file(config_filename);

    if (!config_file.is_open()) {
        std::cerr << "Failed to open config file: " << config_filename << std::endl;
        exit(1);
    }

    std::string line;
    while (getline(config_file, line)) {
        std::istringstream line_stream(line);
        std::string key;
        if (getline(line_stream, key, '=')) {
            std::string value;
            if (getline(line_stream, value)) {
                if (key == "start_label_address") {
                    config.start_label_address = static_cast<uint16_t>(std::stoi(value, nullptr, 16));
                } else if (key == "output_256_bytes_boot_sector") {
                    config.output_256_bytes_boot_sector = (value == "true");
                } else if (key == "output_4kb_flash") {
                    config.output_4_kb_flash = (value == "true");
                }
            }
        }
    }

    config_file.close();
    return config;
}