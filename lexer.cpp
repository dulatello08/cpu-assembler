//
// Created by Dulat S on 2/1/24.
//

#include "lexer.h"


Lexer::Lexer()
    : labelPattern("^[_a-zA-Z]\\w*"),
      directivePattern("^\\..*"),
      instructionPattern("^[A-Z]{3}"),
      registerPattern("\\$\\d{1,2}"),
      immediatePattern("#\\d+"),
      hexPattern("#\\$[0-9A-Fa-f]+"),
      binaryPattern("#%[01]+"),
      decimalPattern("\\d+"),
      commentPattern(";.*$") {
}

void Lexer::firstPass(const std::vector<std::string>& lines) {
    // Implementation of firstPass
    int lineNum = 0;
    for (const auto& line : lines) {
        std::smatch match;
        if (std::regex_search(line, match, labelPattern) && match.position(0) == 0) {
            labelTable[match.str(0)] = lineNum;
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