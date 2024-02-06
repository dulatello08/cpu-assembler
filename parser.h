//
// Created by Dulat S on 2/6/24.
//

#ifndef PARSER_H
#define PARSER_H
#include <main.h>
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

public:
    Parser(const std::vector<Token>& tokens, const Metadata& metadata)
        : tokens(tokens), metadata(metadata) {}

    void parse() {
        // Implement the parsing logic here.
        // This method should fill the relocationTable and prepare objectCode based on the tokens.
    }

    [[nodiscard]] const std::vector<RelocationEntry>& getRelocationTable() const {
        return relocation_table;
    }

    [[nodiscard]] const std::vector<uint8_t>& getObjectCode() const {
        return object_code;
    }

    void addObjectCodeByte(uint8_t byte) {
        object_code.push_back(byte);
    }

    void addRelocationEntry(const std::string& label, uint16_t address) {
        relocation_table.push_back(RelocationEntry{label, address});
    }

    // Additional methods to handle parsing and relocation can be added here.
};

#endif //PARSER_H
