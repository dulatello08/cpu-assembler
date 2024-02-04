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
    Operand,
    Unknown
};

class Token {
public:
    TokenType type;
    std::string lexeme;
    int line;

    Token(TokenType type, std::string  lexeme, int line)
        : type(type), lexeme(std::move(lexeme)), line(line) {}
};

class Lexer {
    std::regex labelPattern;
    std::regex macroPattern;
    std::regex instructionPattern;

public:
    Lexer();
    std::map<std::string, int> labelTable;
    std::map<std::string, int> macroTable;
    std::vector<Token> tokens;

    void firstPass(std::vector<std::string> &lines);

    void lex(const std::vector<std::string>& lines);
};

#endif //LEXER_H
