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

void print_hex_dump(const std::vector<uint8_t>& object_file) {
    if (object_file.empty()) return; // Early return if the input vector is empty

    std::vector<uint8_t> prev_line(16, 0xFF); // Previous line for comparison, initialized to a non-matching value
    bool is_skipping = false;
    size_t prev_line_start = 0; // Offset of the start of the previous line

    for (size_t i = 0; i < object_file.size(); i += 16) {
        // Get the current line of bytes
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
    std::string outputFile = "a.out";

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_files> [-o <output_file>]" << std::endl;
        return 1;
    }

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
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
    // Linking process (placeholder)
    std::cout << "Linking files: ";
    for (const auto& file : inputFiles) {
        std::cout << file << " ";
    }
    std::cout << "Output file: " << outputFile << std::endl;

    // Actual linking logic would go here
    auto o_files_parser = new object_files_parser(inputFiles);

    o_files_parser->validate_all_files();

    o_files_parser->log_label_info();

    auto memory_class = new memory_layout(o_files_parser->object_file_vectors, o_files_parser->label_info_per_file, o_files_parser->relocation_info_per_file);

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