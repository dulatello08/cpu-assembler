//
// Created by Dulat S on 2/1/24.
//

#ifndef LEXER_H
#define LEXER_H

#include <map>
#include <regex>
#include <string>

enum class TokenType {
    Label,
    Instruction,
    Register,
    Operand2,
    Unknown
};

class Token {
public:
    TokenType type;
    std::string lexeme;
    uint16_t data;

    Token(TokenType type, std::string  lexeme, uint16_t data)
        : type(type), lexeme(std::move(lexeme)), data(data) {}
};

class Lexer {
    std::regex labelPattern;
    std::regex macroPattern;
    std::regex instructionPattern;
    std::regex commentPattern;
    std::regex operandPattern;

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
