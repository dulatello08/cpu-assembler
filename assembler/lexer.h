#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <cctype>

// Token types for classification.
enum class TokenType {
    Label,
    Instruction,
    Operand,
    EndOfLine,
    Unknown
};

// Operand subtypes for more detailed classification.
enum class OperandSubtype {
    Register,
    Immediate,     // e.g., #0x100, #12
    Memory,        // e.g., [0x100], [#0x455], etc.
    OffsetMemory,  // e.g., [reg + #0xNNNN]
    LabelReference,
    Unknown
};

// Structure representing a single token.
struct Token {
    std::string lexeme;      // The raw text.
    TokenType type;          // General token type.
    OperandSubtype subtype;  // More detailed classification.
    std::string data;        // Numeric value, label name, etc.
};

class Lexer {
public:
    // First pass: collects macros and labels.
    void firstPass(const std::vector<std::string>& lines);

    // Second pass: tokenizes the input lines.
    std::vector<Token> secondPass(const std::vector<std::string>& lines);

    /**
     * @brief Retrieve the macro table.
     *
     * @return A reference to the map storing macros.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::string>& getMacroTable() const { return macroTable; }

    /**
     * @brief Retrieve the label table.
     *
     * @return A reference to the map storing label positions.
     */
    [[nodiscard]] const std::unordered_map<std::string, size_t>& getLabelTable() const { return labelTable; }

private:
    std::unordered_map<std::string, std::string> macroTable; // Stores macros.
    std::unordered_map<std::string, size_t> labelTable;      // Maps labels to line numbers.

    /**
     * @brief Trim leading and trailing whitespace from a string.
     *
     * @param s The string to trim.
     */
    static inline void trim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    /**
     * @brief Expand macros in a given line.
     *
     * @param line The line to process.
     * @return The expanded line.
     */
    std::string expandMacros(const std::string& line);

    /**
     * @brief Determine the operand subtype.
     *
     * @param operandText The operand string.
     * @return The corresponding operand subtype.
     */
    OperandSubtype parseOperandSubtype(const std::string &operandText);
};

#endif // LEXER_H