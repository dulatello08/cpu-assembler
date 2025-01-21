//
// Created by gitpod on 2/7/24.
//

#include <limits>
#include <algorithm>
#include "parser.h"
#include "assembler.h"
#include "machine_description.h"
#include "sstream"

void Parser::parse() {
    currentTokenIndex = 0;

    while (currentTokenIndex < tokens.size()) {
        const Token &t = tokens[currentTokenIndex];

        if (t.type == TokenType::Label) {
            currentTokenIndex++;
        } else if (t.type == TokenType::Instruction) {
            try {
                // Parse the instruction and get the mnemonic and specifier syntax.
                parse_instruction();
            } catch (const std::exception &e) {
                std::cerr << e.what() << "\n";
            }
        } else {
            std::cerr << "Unexpected token: " << t.data
                    << " at index " << currentTokenIndex << "\n";
            currentTokenIndex++;
        }
    }
}


void Parser::parse_instruction() {
    // 1) Current token => instruction
    const Token &inst_token = tokens[currentTokenIndex];
    std::string inst_name = inst_token.data; // e.g. "mov", "add", "hlt"
    currentTokenIndex++;

    // 2) Gather operand tokens until next instruction or label
    std::vector<Token> operand_tokens;
    while (currentTokenIndex < tokens.size()) {
        const Token &lookahead = tokens[currentTokenIndex];
        if (lookahead.type == TokenType::Instruction ||
            lookahead.type == TokenType::Label) {
            break; // next instruction or label => stop collecting
        }
        operand_tokens.push_back(lookahead);
        currentTokenIndex++;
    }

    // 3) Find the instruction format
    const InstructionFormat *fmt = find_instruction_format(inst_name.c_str());
    if (!fmt) {
        throw std::runtime_error("Unknown instruction: " + inst_name);
    }

    // We'll attempt to match operandTokens against each specifier's syntax.
    // For example, a specifier might have syntax = "mov %rd, #%immediate".
    const InstructionSpecifier *chosenSpec = nullptr;

    for (size_t i = 0; i < fmt->num_specifiers; ++i) {
        const InstructionSpecifier &spec = fmt->specifiers[i];
        if (match_operands_against_syntax(operand_tokens, spec.syntax)) {
            chosenSpec = &spec;
            break;
        }
    }

    if (!chosenSpec) {
        // None of the specifiers' syntax matched
        throw std::runtime_error(
            "No matching syntax for '" + inst_name + "' with the given operands."
        );
    }

    // Now 'chosenSpec' is the specifier that matched
    // ...
    // next, extract operands from the tokens and encode the instruction
    assemble_instruction(chosenSpec, inst_name, operand_tokens);
}

void Parser::assemble_instruction(const InstructionSpecifier *spec,
                                  const std::string &inst_name,
                                  const std::vector<Token> &operand_tokens) {
    // Push the specifier first.
    object_code.push_back(static_cast<uint8_t>(spec->sp));

    // Get and push the opcode.
    uint8_t opcode = get_opcode_for_instruction(inst_name.c_str());
    object_code.push_back(opcode);

    // Use the external helper to get operand lengths.
    std::vector<uint8_t> operand_lengths = get_operand_lengths(inst_name, spec->sp);
    std::ranges::transform(operand_lengths, operand_lengths.begin(), [](uint8_t len) { return (len + 7) / 8; });

    // Use an index to track the position in operand_lengths.
    size_t length_index = 0;

    // Loop through each operand token.
    for (const auto &tok: operand_tokens) {
        if (tok.subtype == OperandSubtype::OffsetMemory) {
            // Consume two lengths for OffsetMemory operands.
            uint8_t len1 = operand_lengths[length_index++];
            uint8_t len2 = operand_lengths[length_index++];

            // Example data format: "[2 + #8]".
            // Remove '[' and ']' characters.
            std::string content = tok.data.substr(1, tok.data.size() - 2);

            // Split the content on the '+' character.
            auto plusPos = content.find('+');
            std::string baseStr = content.substr(0, plusPos);
            std::string offsetStr = content.substr(plusPos + 1);

            // Lambda to trim whitespace.
            auto trim = [](std::string &s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
            };
            trim(baseStr);
            trim(offsetStr);

            // Process base part.
            // Assuming base part is a register number without a prefix.
            int baseReg = std::stoi(baseStr);
            std::vector<uint8_t> operand_bytes1(len1, 0);
            operand_bytes1[0] = static_cast<uint8_t>(baseReg & 0xFF);
            for (auto byte: operand_bytes1) {
                object_code.push_back(byte);
            }

            // Process offset immediate part.
            int offsetVal = 0;
            if (!offsetStr.empty() && offsetStr[0] == '#') {
                offsetVal = std::stoi(offsetStr.substr(1));
            }
            std::vector<uint8_t> operand_bytes2(len2, 0);
            for (int b = 0; b < len2; ++b) {
                operand_bytes2[b] = (offsetVal >> (8 * (len2 - 1 - b))) & 0xFF;
            }
            for (auto byte: operand_bytes2) {
                object_code.push_back(byte);
            }
        } else {
            // For non-OffsetMemory operands, consume one length.
            uint8_t length = operand_lengths[length_index++];

            std::vector<uint8_t> operand_bytes(length, 0);

            if (tok.subtype == OperandSubtype::Immediate) {
                // Remove '#' and convert immediate value.
                int value = std::stoi(tok.data.substr(1));
                for (int b = 0; b < length; ++b) {
                    operand_bytes[b] = (value >> (8 * (length - 1 - b))) & 0xFF;
                }
            } else if (tok.subtype == OperandSubtype::Register) {
                std::string reg = tok.data;
                size_t pos = reg.find(".H");
                if (pos != std::string::npos) reg = reg.substr(0, pos);
                pos = reg.find(".L");
                if (pos != std::string::npos) reg = reg.substr(0, pos);

                // Assuming registers are represented as numeric strings.
                // For example: "3.H" becomes "3" after trimming.
                int reg_num = std::stoi(reg);
                operand_bytes[0] = static_cast<uint8_t>(reg_num & 0xFF);
            } else if (tok.subtype == OperandSubtype::Memory) {
                // Process normal memory addressing like "[#0xFF]".
                std::string content = tok.data.substr(1, tok.data.size() - 2);
                int value = 0;
                if (!content.empty() && content[0] == '#') {
                    value = std::stoi(content.substr(1), nullptr, 0);
                } else {
                    value = std::stoi(content);
                }
                for (int b = 0; b < length; ++b) {
                    operand_bytes[b] = (value >> (8 * (length - 1 - b))) & 0xFF;
                }
            }

            for (auto byte: operand_bytes) {
                object_code.push_back(byte);
            }
        }
    }
}

bool Parser::match_operands_against_syntax(const std::vector<Token> &operand_tokens,
                                           const std::string &syntax_str) {
    // Example syntaxStr: "mov %rd, #%immediate"
    // We want to split on commas, ignoring the instruction mnemonic at the front if it exists.
    // Then we compare each piece: e.g. "%rd" <-> register operand, "#%immediate" <-> immediate operand, etc.

    // 1) Separate the instruction mnemonic (e.g. "mov") from the operand placeholders
    //    We'll assume the first token in syntaxStr is always the mnemonic.
    //    So let's isolate the substring after the first space:

    auto firstSpace = syntax_str.find(' ');
    if (firstSpace == std::string::npos) {
        // means there's no space => probably no operands
        // so if we have no operand tokens, match is true; else false
        return operand_tokens.empty();
    }

    // substring after the mnemonic
    // e.g. from "mov %rd, #%immediate" => "%rd, #%immediate"
    std::string operandPart = syntax_str.substr(firstSpace + 1);

    // 2) Split that on commas:
    std::vector<std::string> syntaxOperands; {
        std::istringstream iss(operandPart);
        std::string temp;
        while (std::getline(iss, temp, ',')) {
            // trim leading/trailing space
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.front()))) temp.erase(temp.begin());
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.back()))) temp.pop_back();
            syntaxOperands.push_back(temp);
        }
    }

    // If the number of syntax operands doesn't match the number of actual operand tokens, no match
    if (syntaxOperands.size() != operand_tokens.size()) {
        return false;
    }

    // 3) Compare each syntax operand placeholder against the token’s subtype
    for (size_t i = 0; i < syntaxOperands.size(); i++) {
        const std::string &placeholder = syntaxOperands[i];
        const Token &actualToken = operand_tokens[i];

        // e.g. placeholder = "%rd", "#%immediate", "[%mem]", ...
        // check if it’s a register placeholder, immediate placeholder, memory, etc.
        if (!placeholder_matches_token(placeholder, actualToken)) {
            return false; // mismatch
        }
    }

    // If all placeholders matched, return true
    return true;
}

bool Parser::placeholder_matches_token(const std::string &placeholder, const Token &tok) {
    // Handle Offset Memory Addressing
    if (placeholder.find("[%rn + #%offset]") != std::string::npos) {
        return (tok.subtype == OperandSubtype::OffsetMemory);
    }
    // Handle Register Placeholders
    else if (placeholder.find("%rd") != std::string::npos ||
             placeholder.find("%rn") != std::string::npos ||
             placeholder.find("%rd1") != std::string::npos ||
             placeholder.find("%rn1") != std::string::npos) {
        // Ensure the token is a Register
        if (tok.subtype != OperandSubtype::Register) {
            return false;
        }

        // Check for ".H" or ".L" in the placeholder
        bool expectsHigh = placeholder.find(".H") != std::string::npos;
        bool expectsLow = placeholder.find(".L") != std::string::npos;

        // Check for ".H" or ".L" in the token's data
        bool hasHigh = tok.data.find(".H") != std::string::npos;
        bool hasLow = tok.data.find(".L") != std::string::npos;

        if (expectsHigh) {
            return hasHigh;
        } else if (expectsLow) {
            return hasLow;
        } else {
            // Placeholder does not specify ".H" or ".L"
            // Ensure token's data does not contain either
            return !hasHigh && !hasLow;
        }
    }
    // Handle Immediate Operands
    else if (placeholder.find("#%immediate") != std::string::npos) {
        return (tok.subtype == OperandSubtype::Immediate);
    }
    // Handle Normal Memory Addressing
    else if (placeholder.find("[%normAddressing]") != std::string::npos) {
        return (tok.subtype == OperandSubtype::Memory);
    }
    // If no known placeholder matched, return false
    return false;
}

std::vector<uint8_t> Parser::get_operand_lengths(const std::string &inst_name, uint8_t sp) {
    const char *encoding = get_encoding_for_instruction(inst_name.c_str(), sp);
    std::string enc_str(encoding);
    std::vector<uint8_t> lengths;

    std::istringstream iss(enc_str);
    std::string token;
    while (iss >> token) {
        // Remove surrounding brackets if present
        if (!token.empty() && token.front() == '[') token.erase(0, 1);
        if (!token.empty() && token.back() == ']') token.pop_back();

        // Token should now be in the form fieldName(bitWidth)
        auto parenOpen = token.find('(');
        auto parenClose = token.find(')');
        if (parenOpen == std::string::npos || parenClose == std::string::npos || parenClose < parenOpen) {
            continue; // skip malformed tokens
        }

        std::string field_name = token.substr(0, parenOpen);
        std::string width_str = token.substr(parenOpen + 1, parenClose - parenOpen - 1);
        auto width = static_cast<uint8_t>(std::stoi(width_str));

        // Skip non-operand fields like 'sp' or 'opcode'
        if (field_name == "sp" || field_name == "opcode") {
            continue;
        }

        lengths.push_back(width);
    }

    return lengths;
}
