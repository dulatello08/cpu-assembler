//
// Created by gitpod on 2/7/24.
//

#ifndef CPU_ASSEMBLER_PARSER_H
#define CPU_ASSEMBLER_PARSER_H

#include <main.h>
#include <utility>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <iostream>

class Parser {
public:
    struct Metadata {
        std::string compiler_version;
        time_t date_of_compilation;
        std::string source_file_name;
    };

    struct RelocationEntry {
        std::string label;
        uint16_t address; // Address is relative to 0x0

        // Add a constructor that takes two arguments
        RelocationEntry(std::string label, uint16_t address)
            : label(std::move(label)), address(address) {}
    };

private:
    std::vector<Token> tokens;
    size_t currentTokenIndex = 0;
    Metadata metadata;
    std::vector<uint8_t> object_code; // The resultant object code in big endian format
    std::vector<uint8_t> conf;
    void addObjectCodeByte(uint8_t byte) {
        object_code.push_back(byte);
    }

public:
    Parser(const std::vector<Token>& tokens, Metadata metadata, std::vector<uint8_t> conf)
            : tokens(tokens), metadata(std::move(metadata)), conf(std::move(conf)){}

    void parse();
    void processToken(const Token& token);

    [[nodiscard]] const std::vector<uint8_t>& getObjectCode() const {
        return object_code;
    }

    void handleRelocation(const Token &token);
};

#endif //CPU_ASSEMBLER_PARSER_H
