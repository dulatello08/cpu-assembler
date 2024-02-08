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
        std::string date_of_compilation;
        std::string source_file_name;
    };

    struct RelocationEntry {
        std::string label;
        uint16_t address; // Address is relative to 0x0
    };

private:
    std::vector<Token> tokens;
    Metadata metadata;
    std::vector<RelocationEntry> relocation_table;
    std::vector<uint8_t> object_code; // The resultant object code in big endian format
    void addRelocationEntry(const std::string& label, uint16_t address) {
        relocation_table.push_back(RelocationEntry{label, address});
    }

public:
    Parser(const std::vector<Token>& tokens, Metadata metadata)
            : tokens(tokens), metadata(std::move(metadata)) {}

    void parse();
    static void processToken(const Token& token);

    [[nodiscard]] const std::vector<RelocationEntry>& getRelocationTable() const {
        return relocation_table;
    }

    [[nodiscard]] const std::vector<uint8_t>& getObjectCode() const {
        return object_code;
    }

    void addObjectCodeByte(uint8_t byte) {
        object_code.push_back(byte);
    }

    static void handleRelocation(const Token &token);
};

#endif //CPU_ASSEMBLER_PARSER_H
