//
// Created by Dulat S on 2/18/24.
//

#ifndef OBJECT_FILE_PARSER_H
#define OBJECT_FILE_PARSER_H
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <fstream>

class object_files_parser {
public:
    std::vector<std::ifstream> object_files_streams;
    std::vector<std::string> object_files;

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
    static std::pair<int, int> findByteRange(const std::vector<uint8_t>& objectFile);
    static std::vector<uint8_t> readFile(const char* filename) {
        std::ifstream file(filename, std::ios::binary);

        if (!file) {
            throw std::runtime_error("Could not open file: " + std::string(filename));
        }

        // Get the size of the file
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read the file into a vector
        std::vector<uint8_t> vec(fileSize);
        file.read(reinterpret_cast<char*>(vec.data()), fileSize);

        return vec;
    }
private:
    void log_error(const std::string& message, size_t file_index) const {
        std::cerr << "Validation failed: " << message << " in file " << object_files[file_index] << std::endl;
    }
    void log_info(const std::string& message, size_t file_index) const {
        std::cout << "Info: " << message << " in file " << object_files[file_index] << std::endl;
    }
};



#endif //OBJECT_FILE_PARSER_H
