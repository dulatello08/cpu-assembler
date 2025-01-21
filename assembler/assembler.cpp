#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <sstream>

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

    // Optional: store extracted info (e.g. register number, immediate value, label name)
    std::string data;        // e.g., "0x7000", "1", "_start"
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
static void firstPass(const std::vector<std::string>& lines)
{
    // If your assembly lines are of the form:
    //   $MACRO ADDRESS 0x7000
    // then let's capture "ADDRESS" as group(1) and "0x7000" as group(2).
    // That means we look specifically for "$MACRO ..." at the start.
    std::regex macroRegex(R"(^\s*\$(?:MACRO\s+)?([A-Za-z_]\w+)\s+(.+)$)");

    // For a label definition: "myLabel:" at the start.
    // Save the line index in labelTable.
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
            // In this scenario:
            // line => "$MACRO ADDRESS 0x7000"
            // m[1] => "ADDRESS"
            // m[2] => "0x7000"
            std::string macroName  = m[1];
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
    // This captures an optional leading '#' plus a word token (macro name).
    // Example: "ADDRESS" or "#ADDRESS" => if "ADDRESS" is found in macroTable,
    // we replace that substring only (the '#' remains in front).
    static std::regex macroTokenRegex(R"((#?)([A-Za-z_]\w*))");

    std::string expanded = line;
    std::smatch match;
    size_t offset = 0;

    while (std::regex_search(expanded.cbegin() + offset, expanded.cend(), match, macroTokenRegex)) {
        // match[1] is the optional '#'
        // match[2] is the actual macro name
        std::string maybeHash = match[1];  // could be ""
        std::string found     = match[2];  // the macro name

        if (macroTable.find(found) != macroTable.end()) {
            // We'll replace only the macro name portion, leaving the '#' if present.
            // So "found" -> macroTable[found], but keep maybeHash in front.
            // The full match was match[1] + match[2], but we only want to replace the
            // second part with the expansion, i.e. the substring that matched found.
            auto matchPos = match.position(2) + offset;  // position of group(2)
            auto matchLen = match.length(2);             // length of group(2)

            // Replace "found" with macroTable[found]
            expanded.replace(matchPos, matchLen, macroTable[found]);

            // Advance offset to the end of the replaced text
            offset = matchPos + macroTable[found].size();
        } else {
            // Macro not found, move offset past this match
            offset += match.position(0) + match.length(0);
        }
    }
    return expanded;
}

// -----------------------------------------------
// Operand Parsing Logic
// -----------------------------------------------
static OperandSubtype parseOperandSubtype(const std::string &operandText) {
    // Some naive checks:
    // 1) Register: if all digits (e.g. "1", "2") or "r1", "r13"
    // 2) Immediate: if starts with '#'
    // 3) Memory: if bracketed => [0x100], [#0x455], ...
    //    - If there's a plus => offset memory
    // 4) LabelReference: if it starts with alpha or '_'
    // Otherwise unknown.

    if (operandText.size() >= 2 && operandText.front() == '[' && operandText.back() == ']') {
        // Check inside
        std::string inside = operandText.substr(1, operandText.size() - 2);
        // trim inside to avoid confusion with spaces
        std::string insideTrimmed = inside;
        trim(insideTrimmed);

        // If there's a plus, assume offset memory
        if (insideTrimmed.find('+') != std::string::npos) {
            return OperandSubtype::OffsetMemory;
        }
        return OperandSubtype::Memory;
    }
    // Immediate
    if (!operandText.empty() && operandText[0] == '#') {
        return OperandSubtype::Immediate;
    }
    // Check register: all digits => treat as register, or "rNN"
    {
        bool allDigits = !operandText.empty()
                         && std::all_of(operandText.begin(), operandText.end(), ::isdigit);
        if (allDigits) {
            return OperandSubtype::Register;
        }
        static std::regex regRegex(R"(^(r\d+)$)");
        if (std::regex_match(operandText, regRegex)) {
            return OperandSubtype::Register;
        }
    }
    // If it starts alpha or '_', treat as label
    if (!operandText.empty() && (std::isalpha(operandText[0]) || operandText[0] == '_')) {
        return OperandSubtype::LabelReference;
    }

    // Fallback
    return OperandSubtype::Unknown;
}

// -----------------------------------------------
// Second Pass: Tokenize
// -----------------------------------------------
//
// A frequent problem is splitting on whitespace and commas
// but then bracketed expressions get split on plus signs or spaces.
// We'll fix it by using a regex that either matches a bracketed block
// as a single token or else matches a sequence of non-space, non-comma chars.
//
static std::vector<Token> secondPass(const std::vector<std::string>& lines) {
    std::vector<Token> tokens;

    // For a label *by itself*, we already recognized it in pass 1, but we might want a token too.
    // We'll detect it with a simpler pattern: "label:" ignoring spaces.
    std::regex labelDef(R"(^\s*([A-Za-z_]\w*):\s*$)");
    // For instructions (heuristic): something that is alpha or underscore + word-chars
    std::regex instructionDef(R"(^[A-Za-z_]\w*$)");

    // This regex grabs either:
    //   1) (\[.*?\]) bracketed text as one token (lazy `.*?` so it won't skip multiple bracket pairs)
    //   2) or ([^,\s]+) a run of non‐space and non‐comma chars
    //
    // That ensures "[2 + 0xffffff]" is matched as one token, no matter spaces or plus sign inside.
    std::regex tokenRegex(R"((\[.*?\])|([^,\s]+))");

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string rawLine = lines[i];
        // Remove comment
        auto commentPos = rawLine.find(';');
        if (commentPos != std::string::npos) {
            rawLine.erase(commentPos);
        }
        trim(rawLine);
        if (rawLine.empty()) {
            continue;
        }

        // Skip macro lines (we parsed them in pass 1)
        if (!rawLine.empty() && rawLine[0] == '$') {
            continue;
        }

        // Expand macros
        std::string expanded = expandMacros(rawLine);
        trim(expanded);
        if (expanded.empty()) continue;

        // Is this a pure label line? (e.g. "myLabel:")
        {
            std::smatch lm;
            if (std::regex_match(expanded, lm, labelDef)) {
                Token t;
                t.lexeme  = lm[1]; // the label text
                t.type    = TokenType::Label;
                t.subtype = OperandSubtype::Unknown;
                t.data    = lm[1];
                tokens.push_back(t);
                continue;
            }
        }

        // Otherwise, gather tokens with tokenRegex
        std::sregex_iterator iter(expanded.begin(), expanded.end(), tokenRegex);
        std::sregex_iterator end;

        bool firstTokenOfLine = true;
        while (iter != end) {
            // If the first capture group is non‐empty, we got bracketed text
            // else we get the second capture group.
            std::string thisToken;
            if ((*iter)[1].matched) {
                thisToken = (*iter)[1];
            } else {
                thisToken = (*iter)[2];
            }
            trim(thisToken);

            if (firstTokenOfLine) {
                // Check if it looks like an instruction
                if (std::regex_match(thisToken, instructionDef)) {
                    // It's an instruction
                    Token t;
                    t.lexeme  = thisToken;
                    t.type    = TokenType::Instruction;
                    t.subtype = OperandSubtype::Unknown;
                    t.data    = thisToken;
                    tokens.push_back(t);
                    firstTokenOfLine = false;
                } else {
                    // Not recognized as instruction, treat as operand (some archs allow that)
                    Token t;
                    t.lexeme  = thisToken;
                    t.type    = TokenType::Operand;
                    t.subtype = parseOperandSubtype(thisToken);
                    t.data    = thisToken;
                    tokens.push_back(t);
                    firstTokenOfLine = false;
                }
            } else {
                // It's an operand
                Token t;
                t.lexeme  = thisToken;
                t.type    = TokenType::Operand;
                t.subtype = parseOperandSubtype(thisToken);
                t.data    = thisToken;
                tokens.push_back(t);
            }

            ++iter;
        }
    }
    return tokens;
}

// -----------------------------------------------
// Example: main()
// -----------------------------------------------
int main() {
    // Example input lines (simulating an .asm file).
    // Notice we are deliberately using the syntax "$MACRO ADDRESS 0x7000"
    // so that the firstPass can parse out "ADDRESS" => "0x7000".
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
        "mov [2 + 0xffffff],  7",
        "",
        "hlt"
    };

    // First pass: collect macros and labels
    firstPass(lines);

    // Second pass: produce tokens (with macros expanded)
    auto tokens = secondPass(lines);

    // Print the resulting tokens
    for (auto &t : tokens) {
        std::cout << "Lexeme: '" << t.lexeme << "', ";
        switch(t.type) {
            case TokenType::Label:
                std::cout << "Type: Label, ";
                break;
            case TokenType::Instruction:
                std::cout << "Type: Instruction, ";
                break;
            case TokenType::Operand:
                std::cout << "Type: Operand, ";
                break;
            case TokenType::EndOfLine:
                std::cout << "Type: EndOfLine, ";
                break;
            default:
                std::cout << "Type: Unknown, ";
                break;
        }

        if (t.type == TokenType::Operand) {
            switch(t.subtype) {
                case OperandSubtype::Register:
                    std::cout << "OperandSubtype: Register, ";
                    break;
                case OperandSubtype::Immediate:
                    std::cout << "OperandSubtype: Immediate, ";
                    break;
                case OperandSubtype::Memory:
                    std::cout << "OperandSubtype: Memory, ";
                    break;
                case OperandSubtype::OffsetMemory:
                    std::cout << "OperandSubtype: OffsetMemory, ";
                    break;
                case OperandSubtype::LabelReference:
                    std::cout << "OperandSubtype: LabelReference, ";
                    break;
                default:
                    std::cout << "OperandSubtype: Unknown, ";
                    break;
            }
        }

        std::cout << "Data: '" << t.data << "'\n";
    }

    // Print the label table
    std::cout << "\nLabel Table:\n";
    for (auto &lbl : labelTable) {
        std::cout << "  " << lbl.first << " => line " << lbl.second << "\n";
    }

    // Print the macro table
    std::cout << "\nMacro Table:\n";
    for (auto &m : macroTable) {
        std::cout << "  " << m.first << " => " << m.second << "\n";
    }

    return 0;
}
