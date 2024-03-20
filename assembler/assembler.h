//
// Created by Dulat S on 2/1/24.
//

#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <ctime>
#include <vector>

#define HASH_TABLE_SIZE 4096 // 4KB
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

inline uint8_t getNumOps(const std::string& instruction, std::vector<uint8_t> conf) {
    return conf.at(hash_function(instruction.c_str()) + 1);
}

inline uint8_t getOp1Mode(const std::string& instruction, std::vector<uint8_t> conf) {
    return conf.at(hash_function(instruction.c_str()) + 2);
}

inline time_t get_compile_unix_time() {
    const char *compile_date = __DATE__; // "Mmm dd yyyy"
    const char *compile_time = __TIME__; // "hh:mm:ss"

    struct tm tm{};
    strptime(compile_date, "%b %d %Y", &tm);
    sscanf(compile_time, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

    // Set the fields not set by strptime.
    tm.tm_isdst = -1;  // Not dealing with daylight saving time

    return mktime(&tm); // Convert to Unix time
}

#endif //MAIN_H
