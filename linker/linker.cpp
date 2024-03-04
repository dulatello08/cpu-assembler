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
#include <iomanip>

#include "memory_layout.h"
#include "object_files_parser.h"

linker_config parseLinkerConfig(const std::string& config_filename);
#include <iostream>
#include <iomanip>
#include <vector>
#include <cctype>

void print_hex_dump(const std::vector<uint8_t>& object_file) {
    std::string prev_line;
    bool repeated_line = false;

    for (size_t i = 0; i < object_file.size(); ++i) {
        // Print offset at the beginning of each line
        if (i % 16 == 0) {
            if (repeated_line) {
                std::cout << "*\n";
                repeated_line = false;
            }
            std::cout << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
        }

        // Print the element in hex format
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(object_file[i]) << " ";

        // Print ASCII representation at the end of each line
        if ((i + 1) % 16 == 0 || i + 1 == object_file.size()) {
            // Calculate the number of spaces needed to align the ASCII output
            int spaces_needed = static_cast<int>((16 - ((i + 1) % 16)) % 16);
            std::string current_line = std::string(spaces_needed * 3, ' ') + "|";

            // Print ASCII characters
            for (size_t j = i - (i % 16); j <= i; ++j) {
                char c = static_cast<char>(object_file[j]);
                current_line += (std::isprint(c) ? c : '.');
            }
            current_line += "|\n";

            // Check if the current line is the same as the previous line
            if (current_line == prev_line) {
                repeated_line = true;
            } else {
                std::cout << current_line;
                prev_line = current_line;
            }
        }
    }

    // Print the final asterisk if the last line was repeated
    if (repeated_line) {
        std::cout << "*\n";
    }
}

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

    linker_config linker_config = parseLinkerConfig(configFile);

    o_files_parser->pre_relocate_all_files();
    o_files_parser->log_label_info();

    auto memory_class = new memory_layout(o_files_parser->object_file_vectors);

    print_hex_dump(memory_class->memory);


    delete o_files_parser;
    delete memory_class;
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