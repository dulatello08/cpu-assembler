//
// Created by Dulat S on 2/1/24.
//

#include "lexer.h"
#include <iostream>

auto parseOperand(const std::string& operand) -> uint16_t {
    if (operand.empty()) {
        throw std::invalid_argument("Operand too short");
    }

    // Determine the format based on the prefix
    if (operand[0] == '0') {
        switch (operand[1]) {
            case 'x': // Hexadecimal
            case 'X':
                return static_cast<uint16_t>(std::strtoul(operand.substr(2).c_str(), nullptr, 16));
            case 'd': // Decimal
            case 'D':
                return static_cast<uint16_t>(std::strtoul(operand.substr(2).c_str(), nullptr, 10));
            case 'b': // Binary
            case 'B':
                // Custom binary parsing needed as std::strtoul doesn't support '0b' prefix
                return static_cast<uint16_t>(std::strtoul(operand.substr(2).c_str(), nullptr, 2));
            case 'c': // ASCII characters
            case 'C': {
                if (operand.length() == 3) {
                    // Single character
                    return static_cast<uint16_t>(operand[2]);
                }
                if (operand.length() == 4) {
                    // Two characters, big endian
                    return static_cast<uint16_t>((operand[2] << 8) | operand[4]);
                }
                throw std::invalid_argument("Invalid ASCII format");
            }
            default:
                throw std::invalid_argument("Unknown format specifier");
        }
    }
    // No format specifier - assume hexadecimal
    return static_cast<uint16_t>(std::strtoul(operand.c_str(), nullptr, 16));
}


Lexer::Lexer()
        : labelPattern(R"(^\.(\S+))"),
          macroPattern(R"(^([A-Za-z_]\w*)\s*=\s*\$([0-9A-Xa-x]+))"),
          instructionPattern(R"(^\s*([A-Z]{3,}))"), // Matches instruction mnemonics.
          operandPattern(R"(\s+([^;,\s]+))"), // Matches operands and registers.
          commentPattern(";.*$") // Matches comments for removal.
{}

void Lexer::firstPass(std::vector<std::string> &lines) {
    int lineNum = 0;
    for (auto& line : lines) {
        std::smatch match;
        // Check for macros first
        if (std::regex_match(line, match, macroPattern)) {
            macroTable[match.str(1)] = static_cast<int>(std::strtoul(match.str(2).c_str(), nullptr, 0));
            lines[lineNum] = "";
        }
        // Then check for labels
        else if (std::regex_match(line, match, labelPattern)) {
            labelTable[match.str(1)] = lineNum;
        }
        else {
            // Expand macros in the line if not a macro or label definition
            for (auto& macro : macroTable) {
                auto searchPattern = std::regex("\\$" + macro.first); // Pattern to find macro usage
                char hexStr[10]; // Buffer for the hexadecimal string
                std::snprintf(hexStr, 10, "#0x%04x", macro.second); // Convert integer to hex string
                line = std::regex_replace(line, searchPattern, hexStr);
            }
        }
        lineNum++;
    }
}


void Lexer::lex(const std::vector<std::string>& lines) {
    std::regex operandPattern(R"(\s+([^;,\s]+))"); // Matches operands and registers.
    for (const auto& line : lines) {
        //std::cout << "Processing line " << lineNum << ": " << line << std::endl;

        // Remove comments from the line to simplify parsing.
        std::string processedLine = std::regex_replace(line, commentPattern, "");
        std::smatch match;

        // Try to match an instruction at the beginning of the line.
        if (std::regex_search(processedLine, match, instructionPattern) && match.size() > 1) {
            tokens.emplace_back(TokenType::Instruction, match[1].str(), 0);
            //std::cout << "Found instruction: " << match[1].str() << std::endl;
            // Assuming the instruction has already been matched and processed here
            auto operandsStart = match[0].length();
            auto operandsStr = processedLine.substr(operandsStart);
            std::sregex_iterator iter(operandsStr.begin(), operandsStr.end(), operandPattern);
            std::sregex_iterator end;
            while (iter != end) {
                for (unsigned i = 1; i < iter->size(); ++i) {
                    std::string operand = (*iter)[i].str();
                    // Classify and create tokens for each operand
                    classifyAndCreateToken(operand);
                }
                ++iter;
            }
        }
    }
}

void Lexer::classifyAndCreateToken(const std::string& operand) {
    if (std::regex_match(operand, std::regex(R"(^[0-9A-D])"))) { // Register, 0-D hex
        tokens.emplace_back(TokenType::Register, operand, static_cast<uint16_t>(std::strtoul(operand.c_str(), nullptr, 16)));
    } else if (std::regex_match(operand, std::regex(R"(^#[0-9a-fA-FxXbBdDcC]+)"))) { // Operand 2
        tokens.emplace_back(TokenType::Operand2, operand, parseOperand(operand.substr(1)));
    } else if (std::regex_match(operand, std::regex(R"(^[a-zA-Z_][a-zA-Z_0-9]*$)"))) { // Label
        //get label line number
        auto labelLineNum = static_cast<uint16_t>(labelTable[operand]);
        tokens.emplace_back(TokenType::Label, operand, labelLineNum);
    } else {
        tokens.emplace_back(TokenType::Unknown, operand, 0);
    }
    // std::cout << "Found operand: " << operand << " as ";
    // switch (tokens.back().type) {
    //     case TokenType::Register: std::cout << "Register"; break;
    //     case TokenType::Operand2: std::cout << "Operand2"; break;
    //     case TokenType::Label: std::cout << "Label"; break;
    //     default: std::cout << "Unknown";
    // }
    // std::cout << std::endl;
}