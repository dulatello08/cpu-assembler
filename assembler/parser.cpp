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
        }
        else if (t.type == TokenType::Instruction) {
            try {
                // Parse the instruction and get the mnemonic and specifier syntax.
                parse_instruction();
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << "\n";
            }
        }
        else {
            std::cerr << "Unexpected token: " << t.data
                      << " at index " << currentTokenIndex << "\n";
            currentTokenIndex++;
        }
    }
}


void Parser::parse_instruction()
{
    // 1) Current token => instruction
    const Token &inst_token = tokens[currentTokenIndex];
    std::string inst_name = inst_token.data; // e.g. "mov", "add", "hlt"
    currentTokenIndex++;

    // 2) Gather operand tokens until next instruction or label
    std::vector<Token> operand_tokens;
    while (currentTokenIndex < tokens.size()) {
        const Token &lookahead = tokens[currentTokenIndex];
        if (lookahead.type == TokenType::Instruction ||
            lookahead.type == TokenType::Label)
        {
            break; // next instruction or label => stop collecting
        }
        operand_tokens.push_back(lookahead);
        currentTokenIndex++;
    }

    // 3) Find the instruction format
    const InstructionFormat* fmt = find_instruction_format(inst_name.c_str());
    if (!fmt) {
        throw std::runtime_error("Unknown instruction: " + inst_name);
    }

    // We'll attempt to match operandTokens against each specifier's syntax.
    // For example, a specifier might have syntax = "mov %rd, #%immediate".
    const InstructionSpecifier* chosenSpec = nullptr;

    for (size_t i = 0; i < fmt->num_specifiers; ++i) {
        const InstructionSpecifier& spec = fmt->specifiers[i];
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
}

bool Parser::match_operands_against_syntax(const std::vector<Token> &operandTokens,
                                        const std::string &syntaxStr)
{
    // Example syntaxStr: "mov %rd, #%immediate"
    // We want to split on commas, ignoring the instruction mnemonic at the front if it exists.
    // Then we compare each piece: e.g. "%rd" <-> register operand, "#%immediate" <-> immediate operand, etc.

    // 1) Separate the instruction mnemonic (e.g. "mov") from the operand placeholders
    //    We'll assume the first token in syntaxStr is always the mnemonic.
    //    So let's isolate the substring after the first space:

    auto firstSpace = syntaxStr.find(' ');
    if (firstSpace == std::string::npos) {
        // means there's no space => probably no operands
        // so if we have no operand tokens, match is true; else false
        return operandTokens.empty();
    }

    // substring after the mnemonic
    // e.g. from "mov %rd, #%immediate" => "%rd, #%immediate"
    std::string operandPart = syntaxStr.substr(firstSpace + 1);

    // 2) Split that on commas:
    std::vector<std::string> syntaxOperands;
    {
        std::istringstream iss(operandPart);
        std::string temp;
        while (std::getline(iss, temp, ',')) {
            // trim leading/trailing space
            while (!temp.empty() && isspace((unsigned char)temp.front())) temp.erase(temp.begin());
            while (!temp.empty() && isspace((unsigned char)temp.back()))   temp.pop_back();
            syntaxOperands.push_back(temp);
        }
    }

    // If the number of syntax operands doesn't match the number of actual operand tokens, no match
    if (syntaxOperands.size() != operandTokens.size()) {
        return false;
    }

    // 3) Compare each syntax operand placeholder against the token’s subtype
    for (size_t i = 0; i < syntaxOperands.size(); i++) {
        const std::string &placeholder = syntaxOperands[i];
        const Token &actualToken = operandTokens[i];

        // e.g. placeholder = "%rd", "#%immediate", "[%mem]", ...
        // check if it’s a register placeholder, immediate placeholder, memory, etc.
        if (!placeholder_matches_token(placeholder, actualToken)) {
            return false; // mismatch
        }
    }

    // If all placeholders matched, return true
    return true;
}

bool Parser::placeholder_matches_token(const std::string &placeholder, const Token &tok)
{
    // Handle Offset Memory Addressing
    if (placeholder.find("[%rn + #%offset]") != std::string::npos) {
        return (tok.subtype == OperandSubtype::OffsetMemory);
    }
    // Handle Register Placeholders
    else if (placeholder.find("%rd") != std::string::npos ||
        placeholder.find("%rn") != std::string::npos ||
        placeholder.find("%rd1") != std::string::npos ||
        placeholder.find("%rn1") != std::string::npos)
    {
        // Ensure the token is a Register
        if (tok.subtype != OperandSubtype::Register) {
            return false;
        }

        // Check for ".H" or ".L" in the placeholder
        bool expectsHigh = placeholder.find(".H") != std::string::npos;
        bool expectsLow  = placeholder.find(".L") != std::string::npos;

        // Check for ".H" or ".L" in the token's data
        bool hasHigh = tok.data.find(".H") != std::string::npos;
        bool hasLow  = tok.data.find(".L") != std::string::npos;

        if (expectsHigh) {
            return hasHigh;
        }
        else if (expectsLow) {
            return hasLow;
        }
        else {
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
    const char* encoding = get_encoding_for_instruction(inst_name.c_str(), sp);
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