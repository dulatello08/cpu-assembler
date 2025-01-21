//
// Created by gitpod on 2/7/24.
//

#include <limits>
#include <algorithm>
#include "parser.h"

#include <map>

#include "assembler.h"
#include "machine_description.h"
#include "sstream"

void Parser::parse() {
    currentTokenIndex = 0;
    // Vector to store instructions in the order they are encountered
    std::vector<std::pair<std::string, std::string>> instructionsInOrder;

    while (currentTokenIndex < tokens.size()) {
        const Token &t = tokens[currentTokenIndex];

        if (t.type == TokenType::Label) {
            currentTokenIndex++;
        }
        else if (t.type == TokenType::Instruction) {
            try {
                // Parse the instruction and get the mnemonic and specifier syntax.
                auto [instName, specSyntax] = parseInstruction();
                instructionsInOrder.push_back({instName, specSyntax});
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

    // Output instructions in the order they were encountered with their specifiers.
    for (const auto &inspec : instructionsInOrder) {
        std::cout << inspec.first << ": " << inspec.second << "\n";
    }
}


std::pair<std::string, std::string> Parser::parseInstruction()
{
    // 1) Current token => instruction
    const Token &instToken = tokens[currentTokenIndex];
    std::string instName = instToken.data; // e.g. "mov", "add", "hlt"
    currentTokenIndex++;

    // 2) Gather operand tokens until next instruction or label
    std::vector<Token> operandTokens;
    while (currentTokenIndex < tokens.size()) {
        const Token &lookahead = tokens[currentTokenIndex];
        if (lookahead.type == TokenType::Instruction ||
            lookahead.type == TokenType::Label)
        {
            break; // next instruction or label => stop collecting
        }
        operandTokens.push_back(lookahead);
        currentTokenIndex++;
    }

    // 3) Find the instruction format
    const InstructionFormat* fmt = find_instruction_format(instName.c_str());
    if (!fmt) {
        throw std::runtime_error("Unknown instruction: " + instName);
    }

    // We'll attempt to match operandTokens against each specifier's syntax.
    // For example, a specifier might have syntax = "mov %rd, #%immediate".
    const InstructionSpecifier* chosenSpec = nullptr;

    for (size_t i = 0; i < fmt->num_specifiers; ++i) {
        const InstructionSpecifier& spec = fmt->specifiers[i];
        if (match_operands_against_syntax(operandTokens, spec.syntax)) {
            chosenSpec = &spec;
            break;
        }
    }

    if (!chosenSpec) {
        // None of the specifiers' syntax matched
        throw std::runtime_error(
            "No matching syntax for '" + instName + "' with the given operands."
        );
    }

    // Now 'chosenSpec' is the specifier that matched
    // ...
    // (Do whatever you want next, like generating object code, etc.)
    return {instName, chosenSpec->syntax};
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

    // If the number of syntax operands doesn’t match the number of actual operand tokens, no match
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