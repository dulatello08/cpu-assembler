//
// Created by Dulat S on 2/6/24.
//

#include "parser.h"

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), machineCode(nullptr), machineCodeSize(0) {}

void Parser::parse() {
    // Initialize machine code array, determine size based on tokens if needed
    // For simplicity, let's assume we allocate a fixed size for demonstration purposes
    machineCodeSize = 1024; // Example size, adjust based on actual needs
    machineCode.reset(new uint8_t[machineCodeSize]);

    for (const auto& token : tokens) {
        processToken(token);
    }
}

std::shared_ptr<uint8_t[]> Parser::getMachineCode() const {
    return machineCode;
}

size_t Parser::getMachineCodeSize() const {
    return machineCodeSize;
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