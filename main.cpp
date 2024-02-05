//
// Created by Dulat S on 2/1/24.
//

#include <cstdio>      // For printf
#include <cstdlib>     // For EXIT_FAILURE
#include <cstring>     // For strcmp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <getopt.h>
#include <cstdint>
#include <lexer.h>

int main(int argc, char* argv[]) {
    std::string input_filename;
    std::string conf_filename;
    std::string output_filename = "program.m";

    int option;
    while ((option = getopt(argc, argv, "i:c:o:")) != -1) {
        switch (option) {
            case 'i':
                input_filename = optarg;
                break;
            case 'c':
                conf_filename = optarg;
                break;
            case 'o':
                output_filename = optarg;
                break;
            default:
                std::printf("Usage: assembler -i <input_file> -c <configuration_file> [-o <output_file>]\n");
                return EXIT_FAILURE;
        }
    }

    if (input_filename.empty() || conf_filename.empty()) {
        std::printf("Usage: assembler -i <input_file> -c <configuration_file> [-o <output_file>]\n");
        return EXIT_FAILURE;
    }

    std::ifstream input_file(input_filename);
    if (!input_file) {
        std::printf("Error opening input file\n");
        return EXIT_FAILURE;
    }

    std::ifstream configuration_file(conf_filename, std::ifstream::binary);
    if (!configuration_file) {
        std::printf("Error opening configuration file\n");
        return EXIT_FAILURE;
    }

    // Get the file size
    configuration_file.seekg(0, std::ios::end);
    long file_size = configuration_file.tellg();
    configuration_file.seekg(0, std::ios::beg);

    // Allocate memory for the data
    std::vector<uint8_t> conf(file_size);

    // Read the data into memory
    if (!configuration_file.read(reinterpret_cast<char*>(conf.data()), file_size)) {
        std::printf("Error reading file\n");
        return EXIT_FAILURE;
    }

    // Process the input file
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input_file, line)) {
        lines.push_back(line);
    }

    auto lexer = new Lexer();

    lexer->firstPass(lines);

    // printf("Label Table:\n");
    // printf("------------\n");
    //
    // // Iterate through the map and print each key-value pair
    // for (const auto& entry : lexer->labelTable) {
    //     printf("Label: %-20s | Value: %d\n", entry.first.c_str(), entry.second);
    // }
    //
    //
    lexer->lex(lines);
    for (const auto& token : lexer->tokens) {
        std::cout << "Token: Type = " << static_cast<int>(token.type) << ", Lexeme = " << token.lexeme << ", Line = " << token.line << std::endl;
    }
    delete lexer;

    // Your processing logic here

    return 0;
}
