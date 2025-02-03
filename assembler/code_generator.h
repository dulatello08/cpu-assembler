//
// Created by gitpod on 2/7/24.
//

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include "lexer.h"
#include "machine_description.h"

class CodeGenerator {
public:
    // Constructor.
    std::unordered_map<std::string, uint16_t> label_table;

    explicit CodeGenerator(std::unordered_map<std::string, uint16_t> label_table):
        label_table(std::move(label_table))
    {};
    struct RelocationEntry {
        std::string label;
        uint32_t address; // Address is relative to 0x0

        // Add a constructor that takes two arguments
        RelocationEntry(std::string label, uint32_t address)
            : label(std::move(label)), address(address) {
        }
    };
    /**
     * Assemble an instruction into object code.
     *
     * @param spec Pointer to the instruction specifier.
     * @param inst_name Instruction mnemonic.
     * @param operand_tokens Tokens for the operands.
     * @param object_code Vector to which the assembled bytes are appended.
     */
    void assemble_instruction(const InstructionSpecifier* spec,
                                const std::string& inst_name,
                                const std::vector<Token>& operand_tokens,
                                std::vector<uint8_t>& object_code);

    /**
     * Get operand field lengths for the given instruction.
     *
     * @param inst_name Instruction mnemonic.
     * @param sp Specifier value.
     * @return Vector of (field name, bit width) pairs.
     */
    static std::vector<std::pair<std::string, uint8_t>> get_operand_lengths(const std::string& inst_name, uint8_t sp);

    /**
     * Build a map from operand placeholders to actual tokens.
     *
     * @param syntax_str The syntax string (e.g., "mov %rd, #%immediate").
     * @param operand_tokens Tokens representing the operands.
     * @return Map from placeholder to Token.
     */
    static std::unordered_map<std::string, Token> build_placeholder_map(const std::string& syntax_str,
                                                                         const std::vector<Token>& operand_tokens);

    /**
     * Parse an offset memory operand like "[2 + #8]".
     *
     * @param token_data The operand string.
     * @return A pair of integers: (base register, offset).
     */
    std::pair<int, int> parse_offset_memory_subfields(const std::string& token_data);

    /**
     * Find the token for a given field.
     *
     * @param field_name The field name.
     * @param placeholder_map Map of placeholders to tokens.
     * @param subFieldOut Output for subfield if needed.
     * @param regSuffixOut Output for register suffix if needed.
     * @return Pointer to the matching token, or nullptr if not found.
     */
    static const Token *find_token_for_field(const std::string &field_name,
                                              const std::unordered_map<std::string, Token> &placeholder_map,
                                              std::string &subFieldOut, std::string &regSuffixOut);

    std::vector<RelocationEntry> relocation_entries;

private:
    // Cache for offset memory operand parsing.
    std::unordered_map<std::string, std::pair<int, int>> offset_memory_cache;
};

#endif // CODE_GENERATOR_H