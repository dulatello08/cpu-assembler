//
// Created by gitpod on 2/7/24.
//
#include "parser.h"
#include "code_generator.h"
#include <iostream>
#include "assembler.h"
#include <sstream>

void Parser::parse() {
    currentTokenIndex = 0;

    while (currentTokenIndex < tokens.size()) {
        const Token& current_token = tokens[currentTokenIndex];
        if (current_token.type == TokenType::Label) {
            // Handle label (e.g., add to symbol table)
            currentTokenIndex++;
        }
        else if (current_token.type == TokenType::Instruction) {
            try {
                parse_instruction();
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << "\n";
                currentTokenIndex++;
            }
        }
        else {
            std::cerr << "Unexpected token: " << current_token.data
                      << " at index " << currentTokenIndex << "\n";
            currentTokenIndex++;
        }
    }
}

void Parser::parse_instruction() {
    // 1) Current token => instruction
    const Token& inst_token = tokens[currentTokenIndex];
    std::string inst_name = inst_token.data; // e.g. "mov", "add", "hlt"
    currentTokenIndex++;

    // 2) Gather operand tokens until next instruction or label
    std::vector<Token> operand_tokens;
    while (currentTokenIndex < tokens.size()) {
        const Token& lookahead = tokens[currentTokenIndex];
        if (lookahead.type == TokenType::Instruction ||
            lookahead.type == TokenType::Label) {
            break;
        }
        operand_tokens.push_back(lookahead);
        currentTokenIndex++;
    }

    // 3) Find the instruction format
    const InstructionFormat* instruction_format = find_instruction_format(inst_name.c_str());
    if (!instruction_format) {
        throw std::runtime_error("Unknown instruction: " + inst_name);
    }

    // Try each specifier until we find one that matches the operand pattern
    const InstructionSpecifier* chosen_spec = nullptr;
    for (size_t i = 0; i < instruction_format->num_specifiers; ++i) {
        const InstructionSpecifier& spec = instruction_format->specifiers[i];
        if (match_operands_against_syntax(operand_tokens, spec.syntax)) {
            chosen_spec = &spec;
            break;
        }
    }
    if (!chosen_spec) {
        throw std::runtime_error(
            "No matching syntax for '" + inst_name + "' with given operands."
        );
    }

    // Delegate assembly to CodeGenerator
    this->code_generator.assemble_instruction(chosen_spec, inst_name, operand_tokens, object_code);
}
bool Parser::match_operands_against_syntax(const std::vector<Token> &operand_tokens,
                                           const std::string &syntax_str) {
    // Example syntaxStr: "mov %rd, #%immediate"
    // We want to split on commas, ignoring the instruction mnemonic at the front if it exists.
    // Then we compare each piece: e.g. "%rd" <-> register operand, "#%immediate" <-> immediate operand, etc.

    // 1) Separate the instruction mnemonic (e.g. "mov") from the operand placeholders
    //    We'll assume the first token in syntaxStr is always the mnemonic.
    //    So let's isolate the substring after the first space:

    auto first_space = syntax_str.find(' ');
    if (first_space == std::string::npos) {
        // means there's no space => probably no operands
        // so if we have no operand tokens, match is true; else false
        return operand_tokens.empty();
    }

    // substring after the mnemonic
    // e.g. from "mov %rd, #%immediate" => "%rd, #%immediate"
    std::string operand_part = syntax_str.substr(first_space + 1);

    // 2) Split that on commas:
    std::vector<std::string> syntax_operands; {
        std::istringstream iss(operand_part);
        std::string temp;
        while (std::getline(iss, temp, ',')) {
            // trim leading/trailing space
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.front()))) temp.erase(temp.begin());
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.back()))) temp.pop_back();
            syntax_operands.push_back(temp);
        }
    }

    // If the number of syntax operands doesn't match the number of actual operand tokens, no match
    if (syntax_operands.size() != operand_tokens.size()) {
        return false;
    }

    // 3) Compare each syntax operand placeholder against the token’s subtype
    for (size_t i = 0; i < syntax_operands.size(); i++) {
        const std::string &placeholder = syntax_operands[i];

        // e.g. placeholder = "%rd", "#%immediate", "[%mem]", ...
        // check if it’s a register placeholder, immediate placeholder, memory, etc.
        if (const Token &actual_token = operand_tokens[i]; !placeholder_matches_token(placeholder, actual_token)) {
            return false; // mismatch
        }
    }

    // If all placeholders matched, return true
    return true;
}
bool Parser::placeholder_matches_token(const std::string &placeholder, const Token &token) {
    // Handle Offset Memory Addressing
    if (placeholder.find("[%rn + #%offset]") != std::string::npos) {
        return (token.subtype == OperandSubtype::OffsetMemory);
    }
    // Handle Register Placeholders
    else if (placeholder.find("%rd") != std::string::npos ||
             placeholder.find("%rn") != std::string::npos ||
             placeholder.find("%rd1") != std::string::npos ||
             placeholder.find("%rn1") != std::string::npos) {
        // Ensure the token is a Register
        if (token.subtype != OperandSubtype::Register) {
            return false;
        }

        // Check for ".H" or ".L" in the placeholder
        bool expects_high = placeholder.find(".H") != std::string::npos;
        bool expects_low = placeholder.find(".L") != std::string::npos;

        // Check for ".H" or ".L" in the token's data
        bool has_high = token.data.find(".H") != std::string::npos;
        bool has_low = token.data.find(".L") != std::string::npos;

        if (expects_high) {
            return has_high;
        } else if (expects_low) {
            return has_low;
        } else {
            // Placeholder does not specify ".H" or ".L"
            // Ensure token's data does not contain either
            return !has_high && !has_low;
        }
             }
    // Handle Immediate Operands
    else if (placeholder.find("#%immediate") != std::string::npos) {
        return (token.subtype == OperandSubtype::Immediate);
    }
    // Handle Normal Memory Addressing
    else if (placeholder.find("[%normAddressing]") != std::string::npos) {
        return (token.subtype == OperandSubtype::Memory);
    }
    // If no known placeholder matched, return false
    return false;
}