#include "lexer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>  // for getopt

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


    return 0;
}