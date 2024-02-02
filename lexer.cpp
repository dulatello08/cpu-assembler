//
// Created by Dulat S on 2/1/24.
//

#include "lexer.h"


Lexer::Lexer()
        : labelPattern(R"(^\.(\S+))"), // Adjust as needed
          directivePattern("^\\..+"),
          instructionPattern("^[A-Z]{3}"),
          registerPattern("\\$\\d{1,2}"),
          immediatePattern("#\\d+"),
          hexPattern("#\\$[0-9A-Fa-f]+"),
          binaryPattern("#%[01]+"),
          decimalPattern("\\d+"),
          commentPattern(";.*$"),
          macroPattern(R"(^([A-Za-z_]\w*)\s*=\s*\$([0-9A-Xa-x]+))") {
}

void Lexer::firstPass(const std::vector<std::string>& lines) {
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
    // Implementation of lex
    int lineNum = 0;
    for (const auto& line : lines) {
        // Implementation details here...
        lineNum++;
    }
}