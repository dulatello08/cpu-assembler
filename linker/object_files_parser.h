//
// Created by Dulat S on 2/18/24.
//

#ifndef OBJECT_FILE_PARSER_H
#define OBJECT_FILE_PARSER_H
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

class object_files_parser {
private:
    std::vector<std::ifstream> object_files_streams;
    std::vector<std::string> object_files;

public:
    explicit object_files_parser(const std::vector<std::string>& object_files): object_files(object_files) {
        for (const auto& file_path : object_files) {
            std::ifstream file_stream(file_path, std::ios::binary | std::ios::in);
            if (file_stream.is_open()) {
                object_files_streams.push_back(std::move(file_stream));
            } else {
                // Handle the case when the file couldn't be opened
                std::cerr << "Error: Unable to open file " << file_path << std::endl;
            }
        }
    }

    // Destructor to close all the open file streams
    ~object_files_parser() {
        for (auto& file_stream : object_files_streams) {
            if (file_stream.is_open()) {
                file_stream.close();
            }
        }
    }

    bool validate_all_files();
};



#endif //OBJECT_FILE_PARSER_H
