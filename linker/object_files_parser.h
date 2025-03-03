//
// Created by Dulat S on 2/18/24.
//

#ifndef OBJECT_FILE_PARSER_H
#define OBJECT_FILE_PARSER_H
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "linker.h"

class object_files_parser {
public:
    std::vector<std::vector<uint8_t>> object_file_vectors;
    std::vector<std::string> object_files;
    std::vector<std::vector<LabelInfo>> label_info_per_file;
    std::vector<std::vector<RelocationInfo>> relocation_info_per_file;

    explicit object_files_parser(const std::vector<std::string>& object_files) : object_files(object_files) {
        for (const auto& file_path : object_files) {
            if (std::ifstream file_stream(file_path, std::ios::binary | std::ios::in); file_stream.is_open()) {
                // Read the contents of the file into a vector
                std::vector<uint8_t> file_content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
                object_file_vectors.push_back(std::move(file_content));
            } else {
                // Handle the case when the file couldn't be opened
                std::cerr << "Error: Unable to open file " << file_path << std::endl;
            }
        }
    }

    // Destructor to close all the open file streams
    ~object_files_parser() = default;

    bool validate_all_files();

    void log_label_info() const;

private:
    void log_error(const std::string& message, size_t file_index) const {
        std::cerr << "Validation failed: " << message << " in file " << object_files[file_index] << std::endl;
    }
    void log_info(const std::string& message, size_t file_index) const {
        std::cout << "Info: " << message << " in file " << object_files[file_index] << std::endl;
    }
};



#endif //OBJECT_FILE_PARSER_H
