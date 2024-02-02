//
// Created by Dulat S on 2/1/24.
//

#ifndef LEXER_H
#define LEXER_H

#include <map>
#include <regex>
#include <string>

enum class TokenType {
    Directive,
    Label,
    Instruction,
    Register,
    Immediate,
    HexNumber,
    BinaryNumber,
    DecimalNumber,
    Comment,
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
    std::vector<Token> tokens;
    std::regex labelPattern;
    std::regex directivePattern;
    std::regex instructionPattern;
    std::regex registerPattern;
    std::regex immediatePattern;
    std::regex hexPattern;
    std::regex binaryPattern;
    std::regex decimalPattern;
    std::regex commentPattern;

public:
    Lexer();
    std::map<std::string, int> labelTable;

    void firstPass(const std::vector<std::string>& lines);

    static void lex(const std::vector<std::string>& lines);
};

#endif //LEXER_H
