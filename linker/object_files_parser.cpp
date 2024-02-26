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

uint64_t htonll(uint64_t hostlonglong) {
    uint32_t high_part = htonl((uint32_t)(hostlonglong >> 32));
    uint32_t low_part = htonl((uint32_t)(hostlonglong & 0xFFFFFFFFULL));
    return (((uint64_t)low_part) << 32) | high_part;
}

#endif // defined(__linux__) && !defined(__APPLE__)


bool object_files_parser::validate_all_files() {
    std::string first_file_version;
    for (size_t i = 0; i < object_files_streams.size(); ++i) {
        auto& file_stream = object_files_streams[i];
        file_stream.seekg(0, std::ios::beg);

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

        for (std::uint16_t j = 0; j < relocation_table_size; ++j) {
            std::string label_name;
            std::getline(file_stream, label_name, '\0');
            if (label_name.empty()) {
                log_error("Missing label name in relocation table", i);
                return false;
            }

            std::uint16_t address;
            file_stream.read(reinterpret_cast<char*>(&address), sizeof(address));
            if (!file_stream) {
                log_error("Could not read label address in relocation table", i);
                return false;
            }
            log_info("Label: " + label_name + ", Address: " + std::to_string(address), i);
        }
    }

    std::cout << "All files validated successfully." << std::endl;
    return true;
}

std::pair<int, int> object_files_parser::findByteRange(const std::vector<uint8_t> &objectFile) {

    std::istringstream file_stream(std::string(objectFile.begin(), objectFile.end()));

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

    std::vector<std::pair<std::string, uint16_t>> labelAddresses;
    for (uint16_t i = 0; i < relocation_table_size; ++i) {
        std::string label_name;
        std::getline(file_stream, label_name, '\0');

        uint16_t address;
        file_stream.read(reinterpret_cast<char*>(&address), sizeof(address));

        labelAddresses.emplace_back(label_name, address);
    }

    // Find the start address of the _start label
    auto start_it = std::find_if(labelAddresses.begin(), labelAddresses.end(),
                                 [](const auto& pair) { return pair.first == "_start"; });

    if (start_it == labelAddresses.end()) {
        throw std::runtime_error("Missing _start label");
    }

    // Find the label immediately following _start
    auto next_it = std::next(start_it);
    if (next_it == labelAddresses.end()) {
        throw std::runtime_error("No label found after _start");
    }

    return std::make_pair(start_it->second, next_it->second);
}
