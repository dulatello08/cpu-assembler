//
// Created by Dulat S on 2/18/24.
//

#include "object_files_parser.h"
#include <arpa/inet.h>
#include <ctime>
#include <sstream>
#include <iostream>
#include <limits>
#include <map>


#if defined(__linux__) && !defined(__APPLE__)
#include <arpa/inet.h>
#include <cstring>

uint64_t htonll(uint64_t hostlonglong) {
    uint32_t high_part = htonl((uint32_t)(hostlonglong >> 32));
    uint32_t low_part = htonl((uint32_t)(hostlonglong & 0xFFFFFFFFULL));
    return (((uint64_t)low_part) << 32) | high_part;
}

#endif // defined(__linux__) && !defined(__APPLE__)



bool object_files_parser::validate_all_files() {
    label_info_per_file.clear(); // Clear any existing data
    std::string first_file_version;
    for (size_t i = 0; i < object_file_vectors.size(); ++i) {
        std::istringstream file_stream(std::string(object_file_vectors[i].begin(), object_file_vectors[i].end()));

        std::string assembler_version;
        std::getline(file_stream, assembler_version, '\0');
        if (assembler_version.empty()) {
            log_error("Missing assembler version", i);
            return false;
        }

        if (i == 0) {
            first_file_version = assembler_version;
        } else if (assembler_version != first_file_version) {
            log_error("Assembler version mismatch", i);
            return false;
        }

        log_info("Assembler version: " + assembler_version, i);

        unsigned long compilation_time;
        file_stream.read(reinterpret_cast<char*>(&compilation_time), sizeof(compilation_time));
        if (!file_stream) {
            log_error("Could not read compilation time", i);
            return false;
        }
        compilation_time = htonll(compilation_time);
        auto temp_time = static_cast<time_t>(compilation_time);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %I:%M:%S %p", std::localtime(&temp_time));
        log_info("Compilation time: " + std::string(buffer), i);

        std::string source_file_name;
        std::getline(file_stream, source_file_name, '\0');
        if (source_file_name.empty()) {
            log_error("Missing source file name", i);
            return false;
        }
        log_info("Source file name: " + source_file_name, i);

        std::uint32_t object_code_length;
        file_stream.read(reinterpret_cast<char*>(&object_code_length), sizeof(object_code_length));
        if (!file_stream) {
            log_error("Could not read object code length", i);
            return false;
        }
        log_info("Object code length: " + std::to_string(object_code_length), i);

        file_stream.seekg(object_code_length, std::ios::cur);
        if (!file_stream) {
            log_error("Object code is shorter than expected", i);
            return false;
        }

        std::uint16_t relocation_table_size;
        file_stream.read(reinterpret_cast<char*>(&relocation_table_size), sizeof(relocation_table_size));
        if (!file_stream) {
            log_error("Could not read relocation table size", i);
            return false;
        }
        log_info("Relocation table size: " + std::to_string(relocation_table_size), i);
        std::vector<LabelInfo> labels_in_file;

        for (std::uint16_t j = 0; j < relocation_table_size; ++j) {
            LabelInfo label_info;
            std::getline(file_stream, label_info.name, '\0');
            if (label_info.name.empty()) {
                log_error("Missing label name in relocation table", i);
                return false;
            }

            file_stream.read(reinterpret_cast<char*>(&label_info.address), sizeof(label_info.address));
            if (!file_stream) {
                log_error("Could not read label address in relocation table", i);
                return false;
            }
            log_info("Label: " + label_info.name + ", Address: " + std::to_string(label_info.address), i);

            labels_in_file.push_back(std::move(label_info));
        }

        label_info_per_file.push_back(std::move(labels_in_file));
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