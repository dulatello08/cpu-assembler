//
// Created by Dulat S on 2/18/24.
//

#include "object_files_parser.h"
#include <arpa/inet.h>


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

        std::int64_t compilation_time;
        file_stream.read(reinterpret_cast<char*>(&compilation_time), sizeof(compilation_time));
        if (!file_stream) {
            log_error("Could not read compilation time", i);
            return false;
        }
        compilation_time = htonl(compilation_time);
        log_info("Compilation time: " + std::to_string(compilation_time), i);

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