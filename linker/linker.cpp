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

void print_hex_dump(const std::vector<uint8_t>& object_file) {
    if (object_file.empty()) return; // Early return if the input vector is empty

    std::vector<uint8_t> prev_line(16, 0xFF); // Previous line for comparison, initialized to a non-matching value
    bool is_skipping = false;
    size_t prev_line_start = 0; // Offset of the start of the previous line

    for (size_t i = 0; i < object_file.size(); i += 16) {
        std::vector<uint8_t> current_line(object_file.begin() + i, object_file.begin() + std::min(object_file.size(), i + 16));

        if (current_line == prev_line) {
            if (!is_skipping) {
                is_skipping = true; // Start skipping
                std::cout << "*\n"; // Print the marker only once at the start of a skip
            }
            // Don't update prev_line or print anything else, just continue to the next iteration
        } else {
            if (is_skipping) {
                // Exiting skip mode, print the range of lines that were skipped
                std::cout << std::setw(8) << std::setfill('0') << std::hex << prev_line_start
                          << " to " << std::setw(8) << std::hex << i - 1 << "\n";
                is_skipping = false;
            }

            // Print offset at the beginning of each line
            std::cout << std::setw(8) << std::setfill('0') << std::hex << i << ": ";

            // Print the hex values for the current line
            for (size_t j = 0; j < current_line.size(); ++j) {
                std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(current_line[j]) << " ";
            }

            // Fill in space if the line is shorter than 16 bytes
            if (current_line.size() < 16) {
                int spaces_needed = 3 * (16 - current_line.size());
                std::cout << std::string(spaces_needed, ' ');
            }

            std::cout << "|";

            // Print ASCII representation
            for (auto& byte : current_line) {
                std::cout << (std::isprint(byte) ? static_cast<char>(byte) : '.');
            }
            std::cout << "|\n";

            prev_line = current_line;
            prev_line_start = i; // Update the start offset of the previous line
        }
    }

    if (is_skipping) {
        // If the file ends with one or more skipped lines, print the final range
        std::cout << std::setw(8) << std::setfill('0') << std::hex << prev_line_start
                  << " to " << std::setw(8) << std::hex << object_file.size() - 1 << "\n";
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

    std::vector<LabelInfo> label_info_per_file_flat;

    for (const auto& inner_vector : o_files_parser->label_info_per_file) {
        label_info_per_file_flat.insert(label_info_per_file_flat.end(), inner_vector.begin(), inner_vector.end());
    }

    auto memory_class = new memory_layout(o_files_parser->object_file_vectors, label_info_per_file_flat);

    print_hex_dump(memory_class->memory);

    std::ofstream output_file(outputFile, std::ios::binary);
    if (output_file.is_open()) {
        output_file.write(reinterpret_cast<const char*>(memory_class->memory.data()), static_cast<std::streamsize>(memory_class->memory.size()));
        output_file.close();
    } else {
        std::cerr << "Failed to open file: " << outputFile << std::endl;
    }


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