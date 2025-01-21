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
#include "machine_description.h"

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
    std::vector<uint8_t> object_code; // The resultant object code in big endian format
    void addObjectCodeByte(uint8_t byte) {
        object_code.push_back(byte);
    }

    std::vector<RelocationEntry> relocation_entries;

public:
    Parser(const std::vector<Token> &tokens, Metadata metadata, std::vector<RelocationEntry> relocation_entries):
          tokens(tokens),
          metadata(std::move(metadata)),
          relocation_entries(std::move(relocation_entries)) {
    }

    void parse();

    void parse_instruction();

    void assemble_instruction(const InstructionSpecifier *spec, const std::string &inst_name,
                              const std::vector<Token> &operand_tokens);

    static bool match_operands_against_syntax(const std::vector<Token> &operand_tokens, const std::string &syntax_str);

    static bool placeholder_matches_token(const std::string &placeholder, const Token &tok);

    static std::vector<uint8_t> get_operand_lengths(const std::string &inst_name, uint8_t sp);

    [[nodiscard]] const std::vector<uint8_t> &getObjectCode() const {
        return object_code;
    }
};

#endif //CPU_ASSEMBLER_PARSER_H
