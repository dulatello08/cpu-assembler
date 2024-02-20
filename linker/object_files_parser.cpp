//
// Created by Dulat S on 2/18/24.
//

#include "object_files_parser.h"


bool object_files_parser::validate_all_files() {
    for (size_t i = 0; i < object_files_streams.size(); ++i) {
        auto& file_stream = object_files_streams[i];
        file_stream.seekg(0, std::ios::beg);

        std::string assembler_version;
        std::getline(file_stream, assembler_version, '\0');
        if (assembler_version.empty()) {
            std::cerr << "Validation failed: Missing assembler version in file " << object_files[i] << std::endl;
            return false;
        }

        std::int64_t compilation_time;
        file_stream.read(reinterpret_cast<char*>(&compilation_time), sizeof(compilation_time));
        if (!file_stream) {
            std::cerr << "Validation failed: Could not read compilation time in file " << object_files[i] << std::endl;
            return false;
        }
        compilation_time = htonl(compilation_time);

        std::string source_file_name;
        std::getline(file_stream, source_file_name, '\0');
        if (source_file_name.empty()) {
            std::cerr << "Validation failed: Missing source file name in file " << object_files[i] << std::endl;
            return false;
        }

        std::uint32_t object_code_length;
        file_stream.read(reinterpret_cast<char*>(&object_code_length), sizeof(object_code_length));
        if (!file_stream) {
            std::cerr << "Validation failed: Could not read object code length in file " << object_files[i] << std::endl;
            return false;
        }

        file_stream.seekg(object_code_length, std::ios::cur);
        if (!file_stream) {
            std::cerr << "Validation failed: Object code is shorter than expected in file " << object_files[i] << std::endl;
            return false;
        }

        std::uint16_t relocation_table_size;
        file_stream.read(reinterpret_cast<char*>(&relocation_table_size), sizeof(relocation_table_size));
        if (!file_stream) {
            std::cerr << "Validation failed: Could not read relocation table size in file " << object_files[i] << std::endl;
            return false;
        }

        for (std::uint16_t j = 0; j < relocation_table_size; ++j) {
            std::string label_name;
            std::getline(file_stream, label_name, '\0');
            if (label_name.empty()) {
                std::cerr << "Validation failed: Missing label name in relocation table in file " << object_files[i] << std::endl;
                return false;
            }

            std::uint16_t address;
            file_stream.read(reinterpret_cast<char*>(&address), sizeof(address));
            if (!file_stream) {
                std::cerr << "Validation failed: Could not read label address in relocation table in file " << object_files[i] << std::endl;
                return false;
            }
        }
    }

    std::cout << "All files validated successfully." << std::endl;
    return true;
}