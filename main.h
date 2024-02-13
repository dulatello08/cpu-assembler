//
// Created by Dulat S on 2/1/24.
//

#ifndef MAIN_H
#define MAIN_H

#include <string>
#define HASH_TABLE_SIZE 1024 // 1KB
#define PRIME_FACTOR 101      // Prime factor for the hash calculation
#define MIX_FACTOR 137       // Mix factor to enhance distribution

// Enhanced hash function
inline unsigned int hash_function(const char *instr_string) {
    unsigned int hash = 0;
    // Assuming instr_string points to at least four characters
    auto ascii_1 = (unsigned int)instr_string[0];
    auto ascii_2 = (unsigned int)instr_string[1];
    auto ascii_3 = (unsigned int)instr_string[2];
    auto ascii_4 = (unsigned int)instr_string[3];

    // Calculate hash based on the enhanced formula
    hash = (((ascii_1 ^ (ascii_2 << 5)) + (ascii_3 ^ (ascii_4 << 5)) * PRIME_FACTOR) ^ MIX_FACTOR) % HASH_TABLE_SIZE;

    return hash;
}

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

inline uint8_t getOpCode(const std::string& instruction, std::vector<uint8_t> conf) {
    return conf.at(hash_function(instruction.c_str()));
}

#endif //MAIN_H
