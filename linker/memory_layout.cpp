//
// Created by Dulat S on 3/3/24.
//

#include "memory_layout.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>

#include "linker.h"

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

std::map<std::string, std::tuple<int, int, int>> memory_layout::find_label_ranges() {
    std::map<std::string, std::tuple<int, int, int>> label_ranges;
    for (size_t file_index = 0; file_index < object_files.size(); ++file_index) {
        std::istringstream file_stream(std::string(object_files[file_index].begin(), object_files[file_index].end()));

        // Skip the assembler version, compilation time, and source file name
        file_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\0'); // Assembler version
        file_stream.ignore(sizeof(unsigned long)); // Compilation time
        file_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\0'); // Source file name

        // Skip object code length
        uint32_t object_code_length;
        file_stream.read(reinterpret_cast<char*>(&object_code_length), sizeof(object_code_length));

        // Skip object code
        file_stream.ignore(object_code_length);

        // Read relocation table
        uint16_t relocation_table_size;
        file_stream.read(reinterpret_cast<char*>(&relocation_table_size), sizeof(relocation_table_size));

        uint16_t prev_address = 0;
        std::string prev_label;
        for (uint16_t i = 0; i < relocation_table_size; ++i) {
            std::string label_name;
            std::getline(file_stream, label_name, '\0');

            uint16_t address;
            file_stream.read(reinterpret_cast<char*>(&address), sizeof(address));

            if (i != 0) {
                label_ranges[prev_label] = std::make_tuple(file_index, prev_address, address);
            }

            prev_label = label_name;
            prev_address = address;
        }

        // Add the last label range
        label_ranges[prev_label] = std::make_tuple(file_index, prev_address, object_code_length);
    }

    return label_ranges;
}

std::map<std::string, std::vector<uint8_t>> memory_layout::extract_label_codes() {
    std::map<std::string, std::vector<uint8_t>> label_codes;

    for (const auto& [label_name, range] : label_ranges) {
        auto [file_index, start, end] = range;

        std::vector<uint8_t> code;
        if (start <= end) {
            code = std::vector<uint8_t>(object_codes[file_index].begin() + start, object_codes[file_index].begin() + end);
        } else {
            // Extract in reverse order if start is greater than end
            code = std::vector<uint8_t>(object_codes[file_index].rbegin() + (object_codes[file_index].size() - start),
                                        object_codes[file_index].rbegin() + (object_codes[file_index].size() - end));
        }

        label_codes[label_name] = std::move(code);
    }

    return label_codes;
}


void memory_layout::write_memory_layout() {
    // Initialize memory with zeros
    memory.resize(0x10000, 0);

    // Write start label code to 0x0 to 0xff
    const auto& start_label_code = label_codes["_start"];
    std::copy(start_label_code.begin(), start_label_code.end(), memory.begin());
    label_memory_addresses["_start"] = 0; // Store the memory address of the _start label

    // Write everything else from 0xf000 to 0xffff, spaced with 4 bytes of 0
    size_t address = 0xf000;
    for (const auto& label_code : label_codes) {
        if (label_code.first != "_start") { // Skip the start label code
            std::copy(label_code.second.begin(), label_code.second.end(), memory.begin() + address);
            label_memory_addresses[label_code.first] = address; // Store the memory address of the label
            address += label_code.second.size() + 4; // Space with 4 bytes of 0
        }
    }
    print_hex_dump(memory);
}

void memory_layout::relocate_memory_layout() {
    for (size_t i = 0; i < memory.size(); ++i) {
        if (memory[i] == 0xea) {
            if (i + 1 < memory.size()) {
                uint8_t label_index = memory[i + 1];
                if (label_index < label_info_per_file.size()) {
                    const auto& label_info = label_info_per_file[label_index];

                    auto it = label_memory_addresses.find(label_info.name);
                    if (it != label_memory_addresses.end()) {
                        uint16_t label_address = it->second;
                        // Convert to big endian
                        memory[i] = static_cast<uint8_t>((label_address >> 8) & 0xFF);
                        memory[i + 1] = static_cast<uint8_t>(label_address & 0xFF);
                    }
                }
            }
            // Skip the next byte as it has already been processed
            ++i;
        }
    }
}