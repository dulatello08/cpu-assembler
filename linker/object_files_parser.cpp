//
// Created by Dulat S on 2/18/24.
//

#include "object_files_parser.h"
#include <arpa/inet.h>
#include <ctime>
#include <sstream>
#include <iostream>
#include <map>


#if defined(__linux__) && !defined(__APPLE__)
#include <cstdint>
#include <endian.h>

inline uint64_t htonll(uint64_t x) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return __builtin_bswap64(x);
#else
    return x;
#endif
}
#endif


bool object_files_parser::validate_all_files() {
    label_info_per_file.clear();       // Clear any existing label data
    relocation_info_per_file.clear();    // Clear any existing relocation data

    // Process each object file
    for (size_t i = 0; i < object_file_vectors.size(); ++i) {
        // Create an input stream from the file vector data
        std::istringstream file_stream(
            std::string(object_file_vectors[i].begin(), object_file_vectors[i].end())
        );

        // --- Read the 32-byte header ---
        // Bytes 0-3: Magic ("LF01")
        char magic[5] = {0};
        file_stream.read(magic, 4);
        if (!file_stream || std::string(magic, 4) != "LF01") {
            log_error("Invalid or missing magic header", i);
            return false;
        }

        // Bytes 4-5: Version (0x0001)
        std::uint16_t version;
        file_stream.read(reinterpret_cast<char*>(&version), sizeof(version));
        version = ntohs(version); // Convert network byte order to host
        if (!file_stream || version != 0x0001) {
            log_error("Unsupported or missing version", i);
            return false;
        }

        // Bytes 6-7: Flags (e.g., 0x0000)
        std::uint16_t flags;
        file_stream.read(reinterpret_cast<char*>(&flags), sizeof(flags));
        flags = ntohs(flags);
        if (!file_stream) {
            log_error("Could not read flags", i);
            return false;
        }

        // Bytes 8-15: Timestamp (current time in microseconds)
        std::uint64_t timestamp;
        file_stream.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
        timestamp = htonll(timestamp); // Ensure proper conversion (assuming htonll is defined)
        if (!file_stream) {
            log_error("Could not read timestamp", i);
            return false;
        }
        // Convert timestamp (microseconds) to seconds for readability
        time_t time_sec = static_cast<long>(timestamp) / 1000000;
        char time_buffer[80];
        std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %I:%M:%S %p", std::localtime(&time_sec));
        log_info("Timestamp: " + std::string(time_buffer), i);

        // Bytes 16-19: Machine Code Length
        std::uint32_t machine_code_length;
        file_stream.read(reinterpret_cast<char*>(&machine_code_length), sizeof(machine_code_length));
        machine_code_length = ntohl(machine_code_length);
        if (!file_stream) {
            log_error("Could not read machine code length", i);
            return false;
        }
        log_info("Machine code length: " + std::to_string(machine_code_length), i);

        // Bytes 20-23: Label Table Offset
        std::uint32_t label_table_offset;
        file_stream.read(reinterpret_cast<char*>(&label_table_offset), sizeof(label_table_offset));
        label_table_offset = ntohl(label_table_offset);
        if (!file_stream) {
            log_error("Could not read label table offset", i);
            return false;
        }
        log_info("Label table offset: " + std::to_string(label_table_offset), i);

        // Bytes 24-27: Relocation Table Offset
        std::uint32_t relocation_table_offset;
        file_stream.read(reinterpret_cast<char*>(&relocation_table_offset), sizeof(relocation_table_offset));
        relocation_table_offset = ntohl(relocation_table_offset);
        if (!file_stream) {
            log_error("Could not read relocation table offset", i);
            return false;
        }
        log_info("Relocation table offset: " + std::to_string(relocation_table_offset), i);

        // Bytes 28-31: Metadata Offset (should be 0 as no metadata is generated)
        std::uint32_t metadata_offset;
        file_stream.read(reinterpret_cast<char*>(&metadata_offset), sizeof(metadata_offset));
        metadata_offset = ntohl(metadata_offset);
        if (!file_stream) {
            log_error("Could not read metadata offset", i);
            return false;
        }
        log_info("Metadata offset: " + std::to_string(metadata_offset), i);

        // --- Validate the Machine Code Section ---
        // The machine code immediately follows the 32-byte header.
        // We skip over it for validation purposes.
        file_stream.seekg(32 + machine_code_length, std::ios::beg);
        if (!file_stream) {
            log_error("Machine code section is incomplete", i);
            return false;
        }

        // --- Read the Label Table ---
        // The label table is located at the offset specified in the header.
        file_stream.seekg(label_table_offset, std::ios::beg);
        if (!file_stream) {
            log_error("Could not seek to label table", i);
            return false;
        }

        // Assume the label table starts with a 32-bit count of labels.
        std::uint32_t label_count;
        file_stream.read(reinterpret_cast<char*>(&label_count), sizeof(label_count));
        label_count = ntohl(label_count);
        if (!file_stream) {
            log_error("Could not read label count", i);
            return false;
        }
        log_info("Label count: " + std::to_string(label_count), i);

        std::vector<LabelInfo> labels_in_file;
        for (std::uint32_t j = 0; j < label_count; ++j) {
            LabelInfo label_info;
            // Read the 4-byte label address.
            file_stream.read(reinterpret_cast<char*>(&label_info.address), sizeof(label_info.address));
            label_info.address = ntohl(label_info.address);
            if (!file_stream) {
                log_error("Could not read label address", i);
                return false;
            }
            // Read a null-terminated label name.
            std::getline(file_stream, label_info.name, '\0');
            if (label_info.name.empty()) {
                log_error("Missing label name in label table", i);
                return false;
            }
            log_info("Label: " + label_info.name + ", Address: " + std::to_string(label_info.address), i);
            labels_in_file.push_back(std::move(label_info));
        }
        label_info_per_file.push_back(std::move(labels_in_file));

        // --- Read the Relocation Table ---
        // The relocation table is located at the offset specified in the header.
        file_stream.seekg(relocation_table_offset, std::ios::beg);
        if (!file_stream) {
            log_error("Could not seek to relocation table", i);
            return false;
        }

        // The relocation table starts with a 32-bit count of relocations.
        std::uint32_t relocation_count;
        file_stream.read(reinterpret_cast<char*>(&relocation_count), sizeof(relocation_count));
        relocation_count = ntohl(relocation_count);
        if (!file_stream) {
            log_error("Could not read relocation count", i);
            return false;
        }
        log_info("Relocation count: " + std::to_string(relocation_count), i);

        std::vector<RelocationInfo> relocations_in_file;
        for (std::uint32_t j = 0; j < relocation_count; ++j) {
            RelocationInfo reloc_info{};
            // Read the 4-byte relocation address.
            file_stream.read(reinterpret_cast<char*>(&reloc_info.address), sizeof(reloc_info.address));
            reloc_info.address = ntohl(reloc_info.address);
            if (!file_stream) {
                log_error("Could not read relocation address", i);
                return false;
            }
            // Read the 2-byte label index.
            file_stream.read(reinterpret_cast<char*>(&reloc_info.absolute_index), sizeof(reloc_info.absolute_index));
            reloc_info.absolute_index = ntohs(reloc_info.absolute_index);
            if (!file_stream) {
                log_error("Could not read relocation label index", i);
                return false;
            }
            log_info("Relocation: Address = " + std::to_string(reloc_info.address) +
                     ", Label Index = " + std::to_string(reloc_info.absolute_index), i);
            relocations_in_file.push_back(reloc_info);
        }
        relocation_info_per_file.push_back(std::move(relocations_in_file));
    }

    std::cout << "All files validated successfully." << std::endl;
    return true;
}

void object_files_parser::pre_relocate_all_files() {
    for (int file_index = static_cast<int>(object_file_vectors.size()) - 1; file_index >= 0; --file_index) {
        auto& code = object_file_vectors[file_index];

        // Extract the original size of the object code
        std::istringstream file_stream(std::string(code.begin(), code.end()));
        std::string assembler_version;
        std::getline(file_stream, assembler_version, '\0');
        unsigned long compilation_time;
        file_stream.read(reinterpret_cast<char*>(&compilation_time), sizeof(compilation_time));
        std::string source_file_name;
        std::getline(file_stream, source_file_name, '\0');
        std::uint32_t original_object_code_length;
        file_stream.read(reinterpret_cast<char*>(&original_object_code_length), sizeof(original_object_code_length));
        size_t header_size = file_stream.tellg();

        size_t original_code_size = code.size();

        for (size_t i = header_size; i < code.size(); ++i) {
            if (code[i] == 0xea) { // Found a label reference
                size_t start_index = i;
                size_t end_index = i + 1;
                while (end_index < code.size() && code[end_index] != '\0') {
                    ++end_index;
                }
                if (end_index == code.size()) {
                    // Null terminator not found - handle error
                    std::cerr << "Error: Null terminator not found after 0xea in file " << object_files[file_index] << std::endl;
                    return;
                }

                std::string label_name(reinterpret_cast<char*>(&code[i + 1]), end_index - (i + 1));
                int label_index = -1;
                size_t absolute_index = 0;
                for (auto& fi : label_info_per_file) {
                    for (auto& li : fi) {
                        if (li.name == label_name) {
                            label_index = static_cast<int>(absolute_index);
                            break;
                        }
                        ++absolute_index;
                    }
                    if (label_index != -1) {
                        break;
                    }
                }
                if (label_index == -1) {
                    // Label not found - handle error
                    std::cerr << "Error: Label '" << label_name << "' not found in file " << object_files[file_index] << std::endl;
                    return;
                }

                // Replace the label reference with 0xea and the index of the label
                code[start_index + 1] = static_cast<uint8_t>(label_index);
                // Remove the label name and null terminator
                code.erase(code.begin() + start_index + 2, code.begin() + end_index + 1);
                i = start_index + 1; // Update the index to continue searching
            }
        }

        // Update the size of the object code in the vector
        std::uint32_t new_object_code_length = original_object_code_length - (original_code_size - code.size());
        std::memcpy(&code[header_size - sizeof(original_object_code_length)], &new_object_code_length, sizeof(new_object_code_length));
    }
}