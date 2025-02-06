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
#include <cstdint>
#include <vector>
#include <arpa/inet.h>  // For ntohl

#include <sstream>
#include <arpa/inet.h>  // for ntohl()

void memory_layout::extract_object_codes() {
    // For each object file with its index.
    for (size_t file_index = 0; file_index < object_files.size(); ++file_index) {
        const auto &file = object_files[file_index];
        // Create a binary input stream from the raw file data.
        std::istringstream file_stream(
            std::string(file.begin(), file.end()),
            std::ios::binary
        );

        // --- Header Layout ---
        // Bytes  0-3: Magic ("LF01")
        // Bytes  4-5: Version (0x0001)
        // Bytes  6-7: Flags.
        // Bytes  8-15: Timestamp.
        // Bytes 16-19: Machine Code Length.
        // Bytes 20-23: Label Table Offset.
        // Bytes 24-27: Relocation Table Offset.
        // Bytes 28-31: Metadata Offset.
        //
        // We only need the machine code length (offset 16).

        // Seek to byte offset 16.
        file_stream.seekg(16, std::ios::beg);
        if (!file_stream) {
            std::cerr << "Error: Could not seek to machine code length field in file index " << file_index << std::endl;
            continue;
        }

        // Read the 4-byte machine code length.
        std::uint32_t machine_code_length;
        file_stream.read(reinterpret_cast<char *>(&machine_code_length), sizeof(machine_code_length));
        if (!file_stream) {
            std::cerr << "Error: Could not read machine code length in file index " << file_index << std::endl;
            continue;
        }
        // Convert from network byte order to host order.
        machine_code_length = ntohl(machine_code_length);

        // The machine code itself starts at offset 32.
        file_stream.seekg(32, std::ios::beg);
        if (!file_stream) {
            std::cerr << "Error: Could not seek to machine code section in file index " << file_index << std::endl;
            continue;
        }

        // Read the machine code bytes into a temporary vector.
        std::vector<uint8_t> temp_code(machine_code_length);
        file_stream.read(reinterpret_cast<char *>(temp_code.data()), machine_code_length);
        if (!file_stream) {
            std::cerr << "Error: Incomplete machine code read in file index " << file_index << std::endl;
            continue;
        }

        // Get the base offset where this file's code will be placed in the final memory buffer.
        size_t base_offset = memory.size();

        // Append the temporary machine code directly into the unified memory buffer.
        memory.insert(memory.end(), temp_code.begin(), temp_code.end());

        // Fix-up label addresses for this file:
        // Each label's original file-relative address is updated by adding the base offset.
        for (auto &label : label_info_per_file[file_index]) {
            label.address += static_cast<int>(base_offset);
        }

        // Fix-up relocation reference addresses for this file:
        // Each relocation's original file-relative ref_address is updated by adding the base offset.
        for (auto &reloc : relocation_info_per_file[file_index]) {
            reloc.address += static_cast<int>(base_offset);
        }
    }
}

void memory_layout::relocate_memory_layout() {
    // Build a mapping from label name to unified address.
    std::map<std::string, uint32_t> unified_label_map;
    for (const auto &labels_in_file : label_info_per_file) {
        for (const auto &label : labels_in_file) {
            unified_label_map[label.name] = label.address;
        }
    }

    // Iterate over each file's relocations.
    for (const auto &relocs_in_file : relocation_info_per_file) {
        for (const auto &reloc : relocs_in_file) {
            // Get the symbol name using the stored label_location indices.
            int file_index = reloc.label_location.first;
            int label_index = reloc.label_location.second;
            if (file_index < 0 || file_index >= static_cast<int>(label_info_per_file.size())) {
                std::cerr << "Error: Invalid file index (" << file_index
                          << ") in relocation entry.\n";
                continue;
            }
            if (label_index < 0 || label_index >= static_cast<int>(label_info_per_file[file_index].size())) {
                std::cerr << "Error: Invalid label index (" << label_index
                          << ") in relocation entry for file " << file_index << ".\n";
                continue;
            }
            std::string symbol = label_info_per_file[file_index][label_index].name;
            auto it = unified_label_map.find(symbol);
            if (it == unified_label_map.end()) {
                std::cerr << "Error: Symbol '" << symbol
                          << "' not found in the unified label table.\n";
                continue;
            }
            uint32_t unified_address = it->second;

            // Make sure we have space in memory to write 4 bytes.
            if (reloc.address + 4 > memory.size()) {
                std::cerr << "Error: Relocation address " << reloc.address
                          << " is out of bounds (memory size: " << memory.size() << ").\n";
                continue;
            }

            // Patch the 4 bytes at the relocation address with the big-endian unified address.
            memory[reloc.address]     = (unified_address >> 24) & 0xFF;
            memory[reloc.address + 1] = (unified_address >> 16) & 0xFF;
            memory[reloc.address + 2] = (unified_address >> 8)  & 0xFF;
            memory[reloc.address + 3] = (unified_address)       & 0xFF;
        }
    }
}