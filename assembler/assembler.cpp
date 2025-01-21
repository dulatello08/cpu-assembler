#include "lexer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>  // for getopt

#include "parser.h"

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

    auto *parser = new Parser(tokens, Parser::Metadata(), std::vector<Parser::RelocationEntry>());
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

    return 0;
}
