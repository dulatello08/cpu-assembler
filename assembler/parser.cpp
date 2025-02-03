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
            // Skip labels, assuming they're handled elsewhere.
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
    const Token& inst_token = tokens[currentTokenIndex];
    std::string inst_name = inst_token.data;
    currentTokenIndex++;

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

    const InstructionFormat* instruction_format = find_instruction_format(inst_name.c_str());
    if (!instruction_format) {
        throw std::runtime_error("Unknown instruction: " + inst_name);
    }

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

    this->code_generator.assemble_instruction(chosen_spec, inst_name, operand_tokens, object_code);
}

bool Parser::match_operands_against_syntax(const std::vector<Token> &operand_tokens,
                                           const std::string &syntax_str) {
    auto first_space = syntax_str.find(' ');
    if (first_space == std::string::npos) {
        return operand_tokens.empty();
    }

    std::string operand_part = syntax_str.substr(first_space + 1);
    std::vector<std::string> syntax_operands;

    {
        std::istringstream iss(operand_part);
        std::string temp;
        while (std::getline(iss, temp, ',')) {
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.front()))) temp.erase(temp.begin());
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.back()))) temp.pop_back();
            syntax_operands.push_back(temp);
        }
    }

    if (syntax_operands.size() != operand_tokens.size()) {
        return false;
    }

    for (size_t i = 0; i < syntax_operands.size(); i++) {
        const std::string &placeholder = syntax_operands[i];

        if (const Token &actual_token = operand_tokens[i]; !placeholder_matches_token(placeholder, actual_token)) {
            return false;
        }
    }

    return true;
}

bool Parser::placeholder_matches_token(const std::string &placeholder, const Token &token) {
    if (placeholder.find("[%rn + #%offset]") != std::string::npos) {
        return (token.subtype == OperandSubtype::OffsetMemory);
    }
    else if (placeholder.find("%rd") != std::string::npos ||
             placeholder.find("%rn") != std::string::npos ||
             placeholder.find("%rd1") != std::string::npos ||
             placeholder.find("%rn1") != std::string::npos) {
        if (token.subtype != OperandSubtype::Register) {
            return false;
        }

        bool expects_high = placeholder.find(".H") != std::string::npos;
        bool expects_low = placeholder.find(".L") != std::string::npos;

        bool has_high = token.data.find(".H") != std::string::npos;
        bool has_low = token.data.find(".L") != std::string::npos;

        if (expects_high) {
            return has_high;
        } else if (expects_low) {
            return has_low;
        } else {
            return !has_high && !has_low;
        }
    }
    else if (placeholder.find("#%immediate") != std::string::npos) {
        return (token.subtype == OperandSubtype::Immediate);
    }
    else if (placeholder.find("[%normAddressing]") != std::string::npos) {
        return (token.subtype == OperandSubtype::Memory);
    }

    return false;
}