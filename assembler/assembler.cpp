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
#include "lexer.h"
#include "parser.h"
#include "object_file_generator.h"
#include <iomanip>
#include <cctype> // For std::isprint

void print_hex_dump(const std::vector<uint8_t>& object_file) {
    for (size_t i = 0; i < object_file.size(); ++i) {
        // Print offset at the beginning of each line
        if (i % 16 == 0) {
            std::cout << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
        }

        // Print the element in hex format
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(object_file[i]) << " ";

        // Print ASCII representation at the end of each line
        if ((i + 1) % 16 == 0 || i + 1 == object_file.size()) {
            // Calculate the number of spaces needed to align the ASCII output
            int spaces_needed = static_cast<int>((16 - ((i + 1) % 16)) % 16);
            std::cout << std::string(spaces_needed * 3, ' ') << "|";

            // Print ASCII characters
            for (size_t j = i - (i % 16); j <= i; ++j) {
                char c = static_cast<char>(object_file[j]);
                std::cout << (std::isprint(c) ? c : '.');
            }
            std::cout << "|\n";
        }
    }
}

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

    std::vector<Parser::RelocationEntry> relocation_entries;

    relocation_entries.reserve(lexer->labelTable.size());
    for (const auto& pair : lexer->labelTable) {
        relocation_entries.emplace_back(pair.first, lexer->lineNumberToAddressMap[pair.second]);
    }
    std::reverse(relocation_entries.begin(), relocation_entries.end());

    Parser::Metadata metadata = {
        "0.1",
        get_compile_unix_time(),
        input_filename
    };

    auto parser = new Parser(lexer->tokens, metadata, conf);

    parser->parse();

    // Iterate through the vector and print each element in hex
    auto object_file_gen = new ObjectFileGenerator(metadata, parser->getObjectCode(), relocation_entries);

    object_file_gen->generate_object_file(output_filename);
    std::vector<uint8_t> object_file = object_file_gen->get_object_file();

    print_hex_dump(object_file);

    delete lexer;
    delete object_file_gen;
    // Your processing logic here

    return 0;
}
