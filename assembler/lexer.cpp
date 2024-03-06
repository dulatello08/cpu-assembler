//
// Created by Dulat S on 2/1/24.
//

#include "lexer.h"
#include <iostream>

#include <stdexcept>
#include <string>
#include <cctype>
#include <cstdint>
#include <cstring>

uint16_t parseHexadecimal(const std::string& operand) {
    unsigned long result = std::strtoul(operand.c_str(), nullptr, 16);
    if (result > 0xFFFF) {
        throw std::out_of_range("Hexadecimal value out of uint16_t range");
    }
    return static_cast<uint16_t>(result);
}

uint16_t parseDecimal(const std::string& operand) {
    unsigned long result = std::strtoul(operand.c_str(), nullptr, 10);
    if (result > 0xFFFF) {
        throw std::out_of_range("Decimal value out of uint16_t range");
    }
    return static_cast<uint16_t>(result);
}

uint16_t parseBinary(const std::string& operand) {
    unsigned long result = std::strtoul(operand.c_str(), nullptr, 2);
    if (result > 0xFFFF) {
        throw std::out_of_range("Binary value out of uint16_t range");
    }
    return static_cast<uint16_t>(result);
}

uint16_t parseOctal(const std::string& operand) {
    unsigned long result = std::strtoul(operand.c_str(), nullptr, 8);
    if (result > 0xFFFF) {
        throw std::out_of_range("Octal value out of uint16_t range");
    }
    return static_cast<uint16_t>(result);
}

uint16_t parseAscii(const std::string& operand) {
    if (operand.length() > 4) {
        throw std::invalid_argument("Invalid ASCII format length");
    }
    uint16_t result = 0;
    for (size_t i = 0; i < operand.length(); ++i) {
        result |= (static_cast<uint16_t>(operand[i]) << ((operand.length() - i - 1) * 8));
    }
    return result;
}

uint16_t parseOperand(const std::string& operand) {
    if (operand.empty()) {
        throw std::invalid_argument("Operand too short");
    }

    if (operand[0] == '0' && operand.size() > 2) {
        switch (operand[1]) {
            case 'x':
            case 'X':
                return parseHexadecimal(operand.substr(2));
            case 'd':
            case 'D':
                return parseDecimal(operand.substr(2));
            case 'b':
            case 'B':
                return parseBinary(operand.substr(2));
            case 'o':
            case 'O':
                return parseOctal(operand.substr(2));
            case 'c':
            case 'C':
                return parseAscii(operand.substr(2));
            default:
                throw std::invalid_argument("Unknown format specifier");
        }
    }

    // Default to hexadecimal if no other format specifier is provided
    return parseHexadecimal(operand);
}

void Lexer::firstPass(std::vector<std::string> &lines) {
    int lineNum = 0;
    uint16_t address = 0;
    for (auto& line : lines) {
        std::smatch match;

        std::cout << line << std::endl;
        // Check for macros first
        if (std::regex_match(line, match, macroPattern)) {
            macroTable[match.str(1)] = static_cast<int>(std::strtoul(match.str(2).c_str(), nullptr, 0));
            lines[lineNum] = "";
        }
            // Then check for labels
        else if (std::regex_match(line, match, labelPattern)) {
            address++;
            labelTable[match.str(1)] = lineNum;
            std::cout << "label" << match.str(1) << std::endl;
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
        std::string processedLine = std::regex_replace(line, commentPattern, "");
        std::smatch match1;
        if (std::regex_search(processedLine, match1, instructionPattern) && match1.size() > 1) {
            address += getNumOps(match1[1].str(), conf) + 1;
        }

        lineNumberToAddressMap[lineNum] = address;
        lineNum++;
    }
}


void Lexer::lex(const std::vector<std::string>& lines) {
    std::regex operandPattern(R"(\s+([^;,\s]+))"); // Matches operands and registers.
    for (const auto& pair : lineNumberToAddressMap) {
        std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }
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
        } else if (std::regex_match(processedLine, match, labelPattern)) {
            tokens.emplace_back(TokenType::Instruction, "NOP", 0);
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
        uint16_t labelLineNum;
        auto it = labelTable.find(operand);
        if (it != labelTable.end()) {
            // Key exists in the map, access its value
            labelLineNum = static_cast<uint16_t>(it->second);
        }
        tokens.emplace_back(TokenType::Label, operand, lineNumberToAddressMap[labelLineNum]);
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