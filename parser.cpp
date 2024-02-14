//
// Created by gitpod on 2/7/24.
//

#include <limits>
#include "parser.h"

void Parser::parse() {
    for (currentTokenIndex = 0; currentTokenIndex < tokens.size(); ++currentTokenIndex) {
        // Ensure that you do not access tokens out of bounds
        if (!tokens.empty()) {
            processToken(tokens[currentTokenIndex]);
        }
    }
}

void Parser::processToken(const Token& token) {
    // Implementation remains the same, just ensure any vector access checks for size
    switch (token.type) {
        case TokenType::Instruction:
            addObjectCodeByte(getOpCode(token.lexeme, conf));
            break;
        case TokenType::Register: {
            uint16_t packedRegisters = token.data << 4; // Assume upper 4 bits for first register
            // Safely check if next token exists and is a Register
            if (currentTokenIndex + 1 < tokens.size() && tokens[currentTokenIndex + 1].type == TokenType::Register) {
                const auto& nextToken = tokens[++currentTokenIndex]; // Safely move to next token
                packedRegisters |= (nextToken.data & 0xF); // Assume lower 4 bits for second register
            }
            addObjectCodeByte(packedRegisters);
            break;
        }
        case TokenType::Operand2:

            break;
        case TokenType::Label:
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