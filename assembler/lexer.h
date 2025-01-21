#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <cctype>
#include <algorithm>

// -----------------------------------------------
// Token Types / Operand Subtypes
// -----------------------------------------------
enum class TokenType {
    Label,
    Instruction,
    Operand,
    EndOfLine,
    Unknown
};

enum class OperandSubtype {
    Register,
    Immediate,     // e.g., #0x100, #12
    Memory,        // e.g., [0x100], [#0x455], ...
    OffsetMemory,  // e.g., [reg + #0xNNNN]
    LabelReference,
    Unknown
};

// -----------------------------------------------
// Token Structure
// -----------------------------------------------
struct Token {
    std::string lexeme;      // Exact text (including brackets, etc.)
    TokenType type;          // High-level type
    OperandSubtype subtype;  // For operands, a more specific classification
    std::string data;        // e.g., "0x7000", "1", "_start"
};

// -----------------------------------------------
// Lexer Class Definition
// -----------------------------------------------
class Lexer {
private:
    std::unordered_map<std::string, std::string> macroTable;
    std::unordered_map<std::string, size_t> labelTable;

    // Utility: Trim Whitespace
    static inline void trim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // Expand Macros in a single line
    std::string expandMacros(const std::string& line);

    // Operand Parsing Logic
    OperandSubtype parseOperandSubtype(const std::string &operandText);

public:
    // First Pass: Collect Macros and Labels
    void firstPass(const std::vector<std::string>& lines);

    // Second Pass: Tokenize
    std::vector<Token> secondPass(const std::vector<std::string>& lines);

    // Optionally add getters for macroTable and labelTable if needed
    const std::unordered_map<std::string, std::string>& getMacroTable() const { return macroTable; }
    const std::unordered_map<std::string, size_t>& getLabelTable() const { return labelTable; }
};

#endif // LEXER_H