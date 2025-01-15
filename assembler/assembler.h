//
// Created by Dulat S on 2/1/24.
//

#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <ctime>
#include <vector>
#include <cstdint>

#define HASH_TABLE_SIZE 4096 // 4KB
#define PRIME_FACTOR 101      // Prime factor for the hash calculation
#define MIX_FACTOR 137       // Mix factor to enhance distribution

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

inline uint8_t getOpCode(const std::string &instr, const std::vector<uint8_t> &conf) {
    for(size_t i=4;i<conf.size();) {
        if(!strncmp((char*)&conf[i+1], instr.c_str(), instr.size())) return conf[i];
        uint32_t sc = *(uint32_t*)&conf[i+1+32], oc = *(uint32_t*)&conf[i+1+32+4+2*sc];
        i += 1 + 32 + 4 + 2*sc + 4 + (32 + 4)*oc;
    }
    return 0xFF;
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
