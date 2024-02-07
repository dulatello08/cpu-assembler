//
// Created by Dulat S on 2/1/24.
//

#ifndef MAIN_H
#define MAIN_H

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

#endif //MAIN_H
