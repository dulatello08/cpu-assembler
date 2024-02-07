//
// Created by gitpod on 2/7/24.
//

#include "parser.h"

void Parser::parse() {
    for (const auto& token : tokens) {
        processToken(token);
    }
}

void Parser::processToken(const Token& token) {
    // Adjusted switch statement to combine consecutive identical branches
    switch (token.type) {
        case TokenType::Instruction:
        case TokenType::Register:
        case TokenType::Operand2:
            // Combined processing for Instruction, Register, and Operand2
            // Assuming these should be handled the same way. Adjust as necessary.
            break;
        case TokenType::Label:
            // Handle label
            handleRelocation(token);
            break;
        case TokenType::Unknown:
        default:
            // Handle unknown token type
            break;
    }
}

void Parser::handleRelocation(const Token& token) {
    // Implementation depends on how relocation information is to be processed
    // This is just a placeholder
}

void Parser::storeInMachineCode(uint16_t data, size_t offset) const {
    // Adjust for potential narrowing conversion
    if (offset > std::numeric_limits<ptrdiff_t>::max()) {
        throw std::overflow_error("Offset exceeds ptrdiff_t max value");
    }

    // Assuming storeInMachineCode is correctly marked const; if it modifies class state, remove const
    machineCode[(long) offset] = static_cast<uint8_t>((data >> 8) & 0xFF); // High byte
    machineCode[(long) offset + 1] = static_cast<uint8_t>(data & 0xFF); // Low byte
}