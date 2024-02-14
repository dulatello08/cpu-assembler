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
#include <parser.h>
#include <iomanip>

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

    auto lexer = new Lexer(conf);

    lexer->firstPass(lines);
    lexer->lex(lines);

    for (const auto& token : lexer->tokens) {
        std::cout << "Token: Type = " << static_cast<int>(token.type) << ", Lexeme = " << token.lexeme << ", Data/Line = " << token.data << std::endl;
    }
    Parser::Metadata metadata = {
        "0.1",
        get_compile_unix_time(),
        input_filename
    };

    auto parser = new Parser(lexer->tokens, metadata, conf);

    parser->parse();

    // Iterate through the vector and print each element in hex
    for (size_t i = 0; i < parser->getObjectCode().size(); ++i) {
        // Print the element in hex format
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(parser->getObjectCode()[i]);

        // Formatting: add a space after every byte for readability
        std::cout << " ";

        // Optional: Add a new line every 16 bytes to mimic traditional hex dump format
        if ((i + 1) % 16 == 0) {
            std::cout << "\n";
        }
    }

    delete lexer;
    // Your processing logic here

    return 0;
}
