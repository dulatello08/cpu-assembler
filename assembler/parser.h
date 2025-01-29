//
// Created by gitpod on 2/7/24.
//

#ifndef CPU_ASSEMBLER_PARSER_H
#define CPU_ASSEMBLER_PARSER_H

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include "lexer.h"

class CodeGenerator;
class Parser {
public:
    struct Metadata {
        std::string compiler_version;
        time_t date_of_compilation;
        std::string source_file_name;
    };

    struct RelocationEntry {
        std::string label;
        uint32_t address; // Address is relative to 0x0

        // Add a constructor that takes two arguments
        RelocationEntry(std::string label, uint32_t address)
            : label(std::move(label)), address(address) {
        }
    };

private:
    size_t currentTokenIndex = 0;
    std::vector<Token> tokens;
    Metadata metadata;
    void addObjectCodeByte(uint8_t byte) {
        object_code.push_back(byte);
    }

    std::vector<RelocationEntry> relocation_entries;
    CodeGenerator& code_generator;

public:
    Parser(const std::vector<Token> &tokens, Metadata metadata, std::vector<RelocationEntry> relocation_entries, CodeGenerator& code_generator):
          tokens(tokens),
          metadata(std::move(metadata)),
          relocation_entries(std::move(relocation_entries)),
          code_generator(code_generator) {
    }
    void parse();
    void parse_instruction();

    static bool match_operands_against_syntax(const std::vector<Token> &operand_tokens, const std::string &syntax_str);

    static bool placeholder_matches_token(const std::string &placeholder, const Token &token);

    std::vector<uint8_t> object_code; // The resultant object code in big endian format
};

#endif //CPU_ASSEMBLER_PARSER_H
