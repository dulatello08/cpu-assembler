//
// Created by gitpod on 2/7/24.
//

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include "lexer.h"
#include "parser.h"
#include "machine_description.h"

class CodeGenerator {
public:
    // Constructor
    CodeGenerator();

    /**
     * @brief Assemble an instruction into object code.
     *
     * @param spec Pointer to the InstructionSpecifier defining the instruction format.
     * @param inst_name The mnemonic name of the instruction (e.g., "mov", "add", "hlt").
     * @param operand_tokens A vector of Tokens representing the operands.
     * @param object_code The vector where the
     *assembled bytes will be appended.
     */
    void assemble_instruction(const InstructionSpecifier* spec,
                              const std::string& inst_name,
                              const std::vector<Token>& operand_tokens,
                              std::vector<uint8_t>& object_code);

    /**
     * @brief Retrieve the operand field lengths based on the instruction and specificity.
     *
     * @param inst_name The mnemonic name of the instruction.
     * @param sp Specificity value associated with the instruction.
     * @return A vector of pairs, each containing a field name and its bit width.
     */
    static std::vector<std::pair<std::string, uint8_t>> get_operand_lengths(const std::string& inst_name, uint8_t sp);

    /**
     * @brief Build a mapping from operand placeholders to actual tokens.
     *
     * @param syntax_str The syntax string defining operand placeholders (e.g., "mov %rd, #%immediate").
     * @param operand_tokens A vector of Tokens representing the operands.
     * @return An unordered_map mapping each placeholder string to its corresponding Token.
     */
    static std::unordered_map<std::string, Token> build_placeholder_map(const std::string& syntax_str,
                                                                 const std::vector<Token>& operand_tokens);

    /**
     * @brief Parse an offset memory operand to extract base register and offset values.
     *
     * @param token_data The string representation of the offset memory operand (e.g., "[2 + #8]").
     * @return A pair where the first element is the base register value and the second is the offset value.
     */
    std::pair<int, int> parse_offset_memory_subfields(const std::string& token_data);

    static const Token *find_token_for_field(const std::string &field_name,
                                             const std::unordered_map<std::string, Token> &placeholder_map,
                                             std::string &subFieldOut, std::string &regSuffixOut);

private:
    // Cache to store parsed offset memory operands for efficiency
    std::unordered_map<std::string, std::pair<int, int>> offset_memory_cache;
};

#endif // CODE_GENERATOR_H