#include "lexer.h"
#include "code_generator.h"
#include "parser.h"
#include "assembler.h"
#include "object_file_generator.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>  // for getopt
#include <vector>
#include <unordered_map>

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;

    int opt;
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -i input_file -o output_file\n";
                return 1;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Input file required.\n";
        return 1;
    }
    if (output_file.empty()) {
        std::cerr << "Output file required.\n";
        return 1;
    }

    // Read the input file line by line.
    std::vector<std::string> lines;
    std::ifstream in(input_file);
    if (!in) {
        std::cerr << "Error opening input file.\n";
        return 1;
    }
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    in.close();

    // Run lexer passes.
    Lexer lexer;
    lexer.firstPass(lines);
    std::vector<Token> tokens = lexer.secondPass(lines);

    // Build the label table from the lexer's data.
    std::unordered_map<std::string, uint16_t> label_table;
    for (const auto& pair : lexer.getLabelTable()) {
        label_table[pair.first] = static_cast<uint16_t>(pair.second);
    }

    // Create the code generator and parser as stack objects.
    CodeGenerator code_generator(label_table);
    Parser parser(tokens, Parser::Metadata(), code_generator);
    parser.parse();

    // Build the object file using ObjectFileGenerator.
    ObjectFileGenerator object_file_generator(
        code_generator.relocation_entries,
        parser.label_address_table,
        parser.object_code
    );
    std::vector<uint8_t> object_file = object_file_generator.build();

    // Write the final object file in binary mode.
    std::ofstream out(output_file, std::ios::binary);
    if (!out) {
        std::cerr << "Error opening output file.\n";
        return 1;
    }
    out.write(reinterpret_cast<const char*>(object_file.data()),
              static_cast<std::streamsize>(object_file.size()));
    out.close();

    return 0;
}