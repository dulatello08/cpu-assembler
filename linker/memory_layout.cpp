//
// Created by Dulat S on 3/3/24.
//

#include "memory_layout.h"

#include <sstream>
#include <fstream>

void memory_layout::extract_object_codes() {
    for (const auto& file : object_files) {
        std::istringstream file_stream(std::string(file.begin(), file.end()));

        // Skip the assembler version, compilation time, and source file name
        file_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\0'); // Assembler version
        file_stream.ignore(sizeof(unsigned long)); // Compilation time
        file_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\0'); // Source file name

        // Read object code length
        uint32_t object_code_length;
        file_stream.read(reinterpret_cast<char*>(&object_code_length), sizeof(object_code_length));

        // Read object code
        std::vector<uint8_t> object_code(object_code_length);
        file_stream.read(reinterpret_cast<char*>(object_code.data()), object_code_length);

        // Store object code
        object_codes.push_back(std::move(object_code));

        // Skip relocation table
        uint16_t relocation_table_size;
        file_stream.read(reinterpret_cast<char*>(&relocation_table_size), sizeof(relocation_table_size));
        file_stream.ignore(relocation_table_size * (sizeof(uint16_t) + std::numeric_limits<std::streamsize>::max()), '\0');
    }
}

std::pair<int, int> memory_layout::find_global_start_range() const {
    // Assume that the first object file contains the _start label
    const auto& file = object_codes.front();

    // Find the start address of the _start label
    auto start_it = std::find(file.begin(), file.end(), '_');
    if (start_it == file.end()) {
        throw std::runtime_error("Missing _start label");
    }

    // Find the label immediately following _start
    auto next_it = std::find(std::next(start_it), file.end(), '_');
    if (next_it == file.end()) {
        throw std::runtime_error("No label found after _start");
    }

    return std::make_pair(std::distance(file.begin(), start_it), std::distance(file.begin(), next_it));
}