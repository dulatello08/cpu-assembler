//
// Created by gitpod on 2/7/24.
//

#include <limits>
#include <algorithm>
#include "parser.h"
#include "assembler.h"
void Parser::parse() {
    for (currentTokenIndex = 0; currentTokenIndex < tokens.size(); ++currentTokenIndex) {
        // Ensure that you do not access tokens out of bounds
        if (!tokens.empty()) {
            processToken(tokens[currentTokenIndex]);
            (*find_instruction_format("nop")).specifiers;
        }
    }
}
