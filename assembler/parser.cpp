//
// Created by gitpod on 2/7/24.
//

#include "parser.h"
#include "code_generator.h"
#include <iostream>
#include "assembler.h"
#include <sstream>

static inline void trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void Parser::parse() {
    currentTokenIndex = 0;

    while (currentTokenIndex < tokens.size()) {
        const Token& current_token = tokens[currentTokenIndex];

        if (current_token.type == TokenType::Label) {
            // Skip labels, assuming they're handled elsewhere.
            label_address_table[current_token.data] = object_code.size();
            currentTokenIndex++;
        }
        else if (current_token.type == TokenType::Instruction && current_token.data == "db") {
            parse_data_definition();
            continue;
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
        const Token &actual_token = operand_tokens[i];

        if (!placeholder_matches_token(placeholder, actual_token)) {
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
        return (token.subtype == OperandSubtype::Memory ||
                token.subtype == OperandSubtype::LabelReference);
    }
    else if (placeholder.find("%label") != std::string::npos) {
        return (token.subtype == OperandSubtype::LabelReference);
    }

    return false;
}

void Parser::parse_data_definition() {
    // Assume the current token is the "db" directive.
    // Move past the directive token.
    currentTokenIndex++;

    // Process all subsequent tokens that are operands.
    while (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == TokenType::Operand) {
        const Token &operandToken = tokens[currentTokenIndex];
        std::string op = operandToken.data;
        trim(op); // Remove any leading/trailing whitespace

        // Check if the operand is a string literal (quoted with " or ')
        if (op.size() >= 2 &&
            ((op.front() == '"' && op.back() == '"') || (op.front() == '\'' && op.back() == '\''))) {
            // Remove the surrounding quotes.
            std::string asciiString = op.substr(1, op.size() - 2);
            // Append each character as a byte.
            for (char ch : asciiString) {
                object_code.push_back(static_cast<uint8_t>(ch));
            }
            } else {
                // Otherwise, assume the operand is a numeric literal.
                try {
                    // std::stoi supports decimal and hexadecimal (if prefixed with "0x")
                    int value = std::stoi(op, nullptr, 0);
                    if (value < 0 || value > 0xFF) {
                        throw std::runtime_error("Data byte value out of range: " + op);
                    }
                    object_code.push_back(static_cast<uint8_t>(value));
                } catch (const std::exception &e) {
                    std::cerr << "Error parsing data byte '" << op << "': " << e.what() << "\n";
                }
            }
        currentTokenIndex++;
    }
}
