#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <fstream>
#include <cctype>
#include <algorithm>

// -----------------------------------------------
// Include the instructions header (not shown here)
// -----------------------------------------------
#include "machine_description.h"

// -----------------------------------------------
// Token Types
// -----------------------------------------------
enum class TokenType {
    Label,
    Instruction,
    Operand,
    Separator,
    EndOfLine,
    Unknown
};

// -----------------------------------------------
// Token Structure
// -----------------------------------------------
struct Token {
    std::string lexeme;
    TokenType type;
    std::string data; // Extra data if needed (e.g., register number, immediate value, etc.)
};

// -----------------------------------------------
// First-Pass Data Structures (Macros / Labels)
// -----------------------------------------------
static std::unordered_map<std::string, std::string> macroTable;
static std::unordered_map<std::string, size_t> labelTable;

// -----------------------------------------------
// Utility: Trim Whitespace
// -----------------------------------------------
static inline void trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// -----------------------------------------------
// First Pass: Collect Macros and Labels
// -----------------------------------------------
static void firstPass(const std::vector<std::string>& lines) {
    std::regex macroRegex(R"(^\s*\$(\w+)\s+(.+)$)");
    std::regex labelRegex(R"(^\s*([A-Za-z_]\w*):)");

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = lines[i];
        // Remove comments
        auto commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line.erase(commentPos);
        }
        trim(line);
        if (line.empty()) continue;

        // Check for macro definition
        std::smatch m;
        if (std::regex_match(line, m, macroRegex)) {
            std::string macroName = m[1];
            std::string macroValue = m[2];
            trim(macroName);
            trim(macroValue);
            macroTable[macroName] = macroValue;
            continue;
        }

        // Check for label definition
        if (std::regex_match(line, m, labelRegex)) {
            std::string labelName = m[1];
            trim(labelName);
            // Store label line index
            labelTable[labelName] = i;
        }
    }
}

// -----------------------------------------------
// Expand Macros in a single line
// -----------------------------------------------
static std::string expandMacros(const std::string& line) {
    // We'll replace macros that appear as plain tokens (like ADDRESS)
    // or inside #ADDRESS, or inside brackets [ADDRESS].
    // A quick approach is to scan for word-like tokens and check if in macroTable
    // Then replace them.
    // For more complex logic, you'd need a more robust parse/replace.
    // Here, we'll do a simple global approach with a regex capturing word characters:
    static std::regex macroTokenRegex(R"((?:#?)([A-Za-z_]\w*))");
    std::string expanded = line;

    std::smatch match;
    size_t offset = 0;
    while (std::regex_search(expanded.cbegin() + offset, expanded.cend(), match, macroTokenRegex)) {
        std::string found = match[1];
        if (macroTable.find(found) != macroTable.end()) {
            // Replace only if it exactly matches (with or without #) but keep # intact
            // If text is "#ADDRESS", the found part is "ADDRESS", so we replace only
            // that chunk. We'll be naive and assume it's a unique substring for this match.
            // Construct the portion to replace.
            auto matchPos = match.position(1) + offset;
            auto matchLen = match.length(1);
            expanded.replace(matchPos, matchLen, macroTable[found]);
            offset = matchPos + macroTable[found].size();
        } else {
            offset += match.position(0) + match.length(0);
        }
    }
    return expanded;
}

// -----------------------------------------------
// Second Pass: Tokenize
// -----------------------------------------------
static std::vector<Token> secondPass(const std::vector<std::string>& lines) {
    std::vector<Token> tokens;
    // Regex to match known patterns
    // 1) Label: captured in first pass, but we still want to produce a token if it appears.
    // 2) Instruction: e.g. mov, add, sub, hlt, ...
    // 3) Operand possibilities:
    //    - registers (one or more digits)
    //    - immediate (#0xNNNN or #NNNN)
    //    - bracketed addressing [something], can be: [register], [immediate], [register + #immediate], etc.
    // 4) Separator: comma
    // 5) End of line is just a concept, might store it or skip it.
    //
    // We'll handle them in a simple approach line by line.

    // Basic pattern piecewise:
    // - label definition:  ^\s*(\w+):
    // - instruction: ^(\w+)
    // - operand: can be bracket expression or immediate or raw numeric
    // - We also skip lines with macro definitions (already processed in pass 1).
    //
    // We'll do a simpler approach: we remove comments, expand macros, then
    // split the line by spaces/commas (with care for bracketed expressions).
    //
    // For a more robust approach, consider a single regex or a small state machine.

    // Patterns used repeatedly
    std::regex labelDef(R"(^\s*([A-Za-z_]\w*):\s*$)");
    std::regex instructionDef(R"(^[A-Za-z_]\w*$)");
    std::regex separatorDef(R"(^\s*,\s*$)");
    // We skip macro definitions in second pass (already handled).
    // If something looks like: [stuff], #stuff, or plain numeric

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string rawLine = lines[i];
        // Remove comment
        auto commentPos = rawLine.find(';');
        if (commentPos != std::string::npos) {
            rawLine.erase(commentPos);
        }
        trim(rawLine);
        if (rawLine.empty()) continue;

        // Skip lines with macros (they were in pass 1).
        if (!rawLine.empty() && rawLine[0] == '$') {
            continue;
        }

        // Expand macros
        std::string line = expandMacros(rawLine);
        trim(line);
        if (line.empty()) continue;

        // Check if there's a label
        {
            std::smatch lm;
            if (std::regex_match(line, lm, labelDef)) {
                Token t;
                t.lexeme = lm[1];
                t.type = TokenType::Label;
                t.data = lm[1];
                tokens.push_back(t);
                continue; // nothing else on this line if it's purely a label
            }
        }

        // We now split the line by spaces and commas except inside brackets:
        // We'll do a small manual parse to handle bracket grouping.
        std::vector<std::string> parts;
        {
            bool inBrackets = false;
            std::string current;
            for (size_t c = 0; c < line.size(); ++c) {
                char ch = line[c];
                if (ch == '[') {
                    inBrackets = true;
                    current.push_back(ch);
                }
                else if (ch == ']') {
                    inBrackets = false;
                    current.push_back(ch);
                }
                else if (!inBrackets && (ch == ' ' || ch == '\t' || ch == ',')) {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                    if (ch == ',') {
                        parts.push_back(",");
                    }
                }
                else {
                    current.push_back(ch);
                }
            }
            if (!current.empty()) {
                parts.push_back(current);
            }
        }

        // The first token on the line might be an instruction or unknown
        bool firstTokenSeen = false;
        for (auto &p : parts) {
            trim(p);
            if (p.empty()) continue;

            if (p == ",") {
                Token t;
                t.lexeme = p;
                t.type = TokenType::Separator;
                tokens.push_back(t);
                continue;
            }

            // Instruction candidate if it's the first non-label token
            if (!firstTokenSeen) {
                // Possibly an instruction
                std::smatch m;
                if (std::regex_match(p, m, instructionDef)) {
                    // We can cross-check against known instructions from instructions.h if desired
                    Token t;
                    t.lexeme = p;
                    t.type = TokenType::Instruction;
                    t.data = p;
                    tokens.push_back(t);
                    firstTokenSeen = true;
                    continue;
                }
                // If it's not recognized as instruction, treat as operand or unknown
            }

            // Operand
            {
                Token t;
                t.lexeme = p;
                t.type = TokenType::Operand;
                t.data = p;
                tokens.push_back(t);
            }
        }

        // End of line token (optional, if desired)
        // Token eol;
        // eol.lexeme = "<EOL>";
        // eol.type = TokenType::EndOfLine;
        // tokens.push_back(eol);
    }

    return tokens;
}

// -----------------------------------------------
// Example: main()
// -----------------------------------------------
int main() {
    // Example input lines (could also be read from a file)
    // For demonstration, we store them directly here.
    std::vector<std::string> lines = {
        "$MACRO ADDRESS 0x7000",
        "",
        "_start:",
        "",
        "mov 1, [#0xa455]",
        "",
        "mov 1, 0",
        "",
        "sub 1, #0x455",
        "",
        "mov [ADDRESS], 1, 0",
        "",
        "mov 2, [#ADDRESS]",
        "",
        "mov 3, 4 [2 + #0x455] ; ignore comments, offset address",
        "",
        "hlt"
    };

    // First pass: collect macros and labels
    firstPass(lines);

    // Second pass: produce tokens
    auto tokens = secondPass(lines);

    // Print the resulting tokens
    for (auto &t : tokens) {
        std::cout << "Lexeme: '" << t.lexeme << "', ";
        switch(t.type) {
            case TokenType::Label:
                std::cout << "Type: Label";
                break;
            case TokenType::Instruction:
                std::cout << "Type: Instruction";
                break;
            case TokenType::Operand:
                std::cout << "Type: Operand";
                break;
            case TokenType::Separator:
                std::cout << "Type: Separator";
                break;
            case TokenType::EndOfLine:
                std::cout << "Type: EndOfLine";
                break;
            default:
                std::cout << "Type: Unknown";
                break;
        }
        std::cout << ", Data: '" << t.data << "'\n";
    }

    // Example: Print the label table
    std::cout << "\nLabel Table:\n";
    for (auto &lbl : labelTable) {
        std::cout << lbl.first << " => line " << lbl.second << "\n";
    }

    // Example: Print the macro table
    std::cout << "\nMacro Table:\n";
    for (auto &m : macroTable) {
        std::cout << m.first << " => " << m.second << "\n";
    }

    return 0;
}