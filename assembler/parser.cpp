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
    // Variable to track the number of operands for the current instruction
    static uint8_t numOpsRemaining = 0;

    if (token.type == TokenType::Instruction) {
        addObjectCodeByte(getOpCode(token.lexeme, conf));
        // Reset numOpsRemaining for the new instruction
        numOpsRemaining = getNumOps(token.lexeme, conf);
        return; // Return early since the Instruction case has been handled
    }

    if (numOpsRemaining > 0) {
        switch (token.type) {
            case TokenType::Register: {
                uint16_t packedRegisters = token.data << 4; // Assume upper 4 bits for first register
                // Check if next token is also a register to pack them together
                if (currentTokenIndex + 1 < tokens.size() && tokens[currentTokenIndex + 1].type == TokenType::Register) {
                    const auto& nextToken = tokens[++currentTokenIndex]; // Move to next token
                    packedRegisters |= (nextToken.data & 0xF); // Assume lower 4 bits for second register
                    // Registers packed together count as one operand
                }
                addObjectCodeByte(packedRegisters);
                numOpsRemaining--; // One operand (or pair of operands) processed
                break;
            }
            case TokenType::Operand2: {
                // Check how many bytes to use for Operand2 based on remaining operands
                if (numOpsRemaining == 1) {
                    // If only one operand left, use one byte of Operand2
                    addObjectCodeByte(token.data & 0xFF);
                } else {
                    // Otherwise, use both bytes
                    addObjectCodeByte(token.data >> 8);
                    addObjectCodeByte(token.data & 0xFF);
                    numOpsRemaining--; // Decrement again as two bytes (two operands) were processed
                }
                numOpsRemaining--; // One operand processed
                break;
            }
            case TokenType::Label:
                handleRelocation(token);
                break;
            case TokenType::Unknown:
            default:
                // Handle unknown token type or any other cases not explicitly handled
                break;
        }
    }

    // After processing a non-instruction token, check if there's a need to adjust for packed registers
    if (numOpsRemaining < 0) {
        // This condition may occur if registers are packed and treated as one operand
        // Adjust logic as needed to ensure consistency in operand counting
        numOpsRemaining = 0;
    }
}

void Parser::handleRelocation(const Token& token) {
    // Implementation depends on how relocation information is to be processed
    addObjectCodeByte(token.data >> 8);
    addObjectCodeByte(token.data & 0xFF);
}