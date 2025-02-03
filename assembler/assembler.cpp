#include "lexer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>  // for getopt

#include "code_generator.h"
#include "parser.h"
#include "assembler.h"

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
    std::string inputFile;
    std::string outputFile;

    int opt;
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch(opt) {
            case 'i':
                inputFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -i inputfile -o outputfile\n";
                return 1;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Input file required.\n";
        return 1;
    }

    // Read input file lines
    std::vector<std::string> lines;
    std::ifstream in(inputFile);
    if (!in) {
        std::cerr << "Error opening input file.\n";
        return 1;
    }
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    in.close();

    // Run lexer passes
    Lexer lexer;
    lexer.firstPass(lines);
    std::vector<Token> tokens = lexer.secondPass(lines);
    std::unordered_map<std::string, uint16_t> label_table;

    for (const auto& pair : lexer.getLabelTable()) {
        label_table[pair.first] = static_cast<uint16_t>(pair.second);
    }

    // Correct allocation of CodeGenerator
    auto *code_generator = new CodeGenerator(label_table);
    auto *parser = new Parser(tokens, Parser::Metadata(), *code_generator);
    parser->parse();

    // Set up output stream
    std::ostream* outStream = &std::cout;
    std::ofstream outFile;
    if (!outputFile.empty()) {
        outFile.open(outputFile);
        if (!outFile) {
            std::cerr << "Error opening output file.\n";
            return 1;
        }
        outStream = &outFile;
    }

    // Output tokens
    for (auto &t : tokens) {
        *outStream << "Lexeme: '" << t.lexeme << "', ";
        switch(t.type) {
            case TokenType::Label:
                *outStream << "Type: Label, ";
                break;
            case TokenType::Instruction:
                *outStream << "Type: Instruction, ";
                break;
            case TokenType::Operand:
                *outStream << "Type: Operand, ";
                break;
            case TokenType::EndOfLine:
                *outStream << "Type: EndOfLine, ";
                break;
            default:
                *outStream << "Type: Unknown, ";
                break;
        }

        if (t.type == TokenType::Operand) {
            switch(t.subtype) {
                case OperandSubtype::Register:
                    *outStream << "OperandSubtype: Register, ";
                    break;
                case OperandSubtype::Immediate:
                    *outStream << "OperandSubtype: Immediate, ";
                    break;
                case OperandSubtype::Memory:
                    *outStream << "OperandSubtype: Memory, ";
                    break;
                case OperandSubtype::OffsetMemory:
                    *outStream << "OperandSubtype: OffsetMemory, ";
                    break;
                case OperandSubtype::LabelReference:
                    *outStream << "OperandSubtype: LabelReference, ";
                    break;
                default:
                    *outStream << "OperandSubtype: Unknown, ";
                    break;
            }
        }

        *outStream << "Data: '" << t.data << "'\n";
    }



    if (outFile.is_open()) {
        outFile.close();
    }
    print_hex_dump(parser->object_code);
    std::cout << "Relocation Entries:\n";
    for (const auto& entry : code_generator->relocation_entries) {
        std::cout << "Label: " << entry.label
                  << ", Address: 0x" << std::hex << entry.address << std::dec << '\n';
    }
    for (const auto &entry : parser->label_address_table) {
        std::cout << "Label: " << entry.first
                  << ", Address: 0x" << std::hex << std::setw(8) << std::setfill('0') << entry.second
                  << std::dec << std::setfill(' ') << "\n";
    }
    delete parser;
    delete code_generator;
    return 0;
}
