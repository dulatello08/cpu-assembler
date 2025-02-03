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

private:
    size_t currentTokenIndex = 0;
    std::vector<Token> tokens;
    Metadata metadata;
    void addObjectCodeByte(uint8_t byte) {
        object_code.push_back(byte);
    }

    CodeGenerator& code_generator;

public:
    Parser(const std::vector<Token> &tokens, Metadata metadata, CodeGenerator& code_generator):
          tokens(tokens),
          metadata(std::move(metadata)),
          code_generator(code_generator) {
    }
    void parse();
    void parse_instruction();

    static bool match_operands_against_syntax(const std::vector<Token> &operand_tokens, const std::string &syntax_str);

    static bool placeholder_matches_token(const std::string &placeholder, const Token &token);

    std::vector<uint8_t> object_code; // The resultant object code in big endian format
    std::unordered_map<std::string, uint32_t> label_address_table;
};

#endif //CPU_ASSEMBLER_PARSER_H
