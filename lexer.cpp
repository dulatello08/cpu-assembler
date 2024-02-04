//
// Created by Dulat S on 2/1/24.
//

#include "lexer.h"
#include <iostream>


Lexer::Lexer()
        : labelPattern(R"(^\.(\S+))"),
          instructionPattern("^[A-Z]{3}"),
          macroPattern(R"(^([A-Za-z_]\w*)\s*=\s*\$([0-9A-Xa-x]+))") {
}

void Lexer::firstPass(std::vector<std::string> &lines) {
    int lineNum = 0;
    for (const auto& line : lines) {
        std::smatch match;
        // Check for macros first
        if (std::regex_search(line, match, macroPattern)) {
            macroTable[match.str(1)] = (int) std::strtoul(match.str(2).c_str(), nullptr, 0);
        }
        // Then check for labels
        else if (std::regex_search(line, match, labelPattern)) {
            labelTable[match.str(1)] = lineNum;
        }
        lineNum++;
    }
}

void Lexer::lex(const std::vector<std::string>& lines) {
    // Regex patterns to identify comments, instructions, and operands.
    std::regex commentRegex(";.*$"); // Matches comments for removal.
    std::regex instructionRegex(R"(^\s*([A-Z]{3,}))"); // Matches instruction mnemonics.
    std::regex operandRegex(R"(\s+([^;,\s]+))"); // Matches operands and registers.

    int lineNum = 1;
    for (const auto& line : lines) {
        std::cout << "Processing line " << lineNum << ": " << line << std::endl;

        // Remove comments from the line to simplify parsing.
        std::string processedLine = std::regex_replace(line, commentRegex, "");
        std::smatch match;

        // Try to match an instruction at the beginning of the line.
        if (std::regex_search(processedLine, match, instructionRegex) && match.size() > 1) {
            tokens.emplace_back(TokenType::Instruction, match[1].str(), lineNum);
            std::cout << "Found instruction: " << match[1].str() << std::endl;

            // Extract operands and registers following the instruction.
            auto operandsStart = match[0].length();
            auto operandsStr = processedLine.substr(operandsStart);
            std::sregex_iterator iter(operandsStr.begin(), operandsStr.end(), operandRegex);
            std::sregex_iterator end;
            while (iter != end) {
                for (unsigned i = 1; i < iter->size(); ++i) {
                    tokens.emplace_back(TokenType::Operand, (*iter)[i].str(), lineNum);
                    std::cout << "Found operand/register: " << (*iter)[i].str() << std::endl;
                }
                ++iter;
            }
        } else {
            // If no instruction is matched, print a debug message.
            std::cout << "No instruction found on line " << lineNum << std::endl;
        }
        lineNum++;
    }
}