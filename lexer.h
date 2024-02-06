//
// Created by Dulat S on 2/1/24.
//

#ifndef LEXER_H
#define LEXER_H

#include <map>
#include <regex>
#include <string>
#include "main.h"

class Lexer {
    std::regex labelPattern;
    std::regex macroPattern;
    std::regex instructionPattern;
    std::regex operandPattern;
    std::regex commentPattern;

public:
    Lexer();
    std::map<std::string, int> labelTable;
    std::map<std::string, int> macroTable;
    std::vector<Token> tokens;

    void firstPass(std::vector<std::string> &lines);

    void lex(const std::vector<std::string>& lines);
    void classifyAndCreateToken(const std::string& operand);
};

#endif //LEXER_H
