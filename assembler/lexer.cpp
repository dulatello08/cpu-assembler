#include "lexer.h"

// -----------------------------------------------
// Expand Macros in a single line
// -----------------------------------------------
std::string Lexer::expandMacros(const std::string& line) {
    static std::regex macroTokenRegex(R"((#?)([A-Za-z_]\w*))");

    std::string expanded = line;
    std::smatch match;
    size_t offset = 0;

    while (std::regex_search(expanded.cbegin() + offset, expanded.cend(), match, macroTokenRegex)) {
        std::string maybeHash = match[1];
        std::string found     = match[2];

        if (macroTable.find(found) != macroTable.end()) {
            auto matchPos = match.position(2) + offset;
            auto matchLen = match.length(2);
            expanded.replace(matchPos, matchLen, macroTable[found]);
            offset = matchPos + macroTable[found].size();
        } else {
            offset += match.position(0) + match.length(0);
        }
    }
    return expanded;
}

// -----------------------------------------------
// Operand Parsing Logic
// -----------------------------------------------
OperandSubtype Lexer::parseOperandSubtype(const std::string &operandText) {
    if (operandText.size() >= 2 && operandText.front() == '[' && operandText.back() == ']') {
        std::string inside = operandText.substr(1, operandText.size() - 2);
        std::string insideTrimmed = inside;
        trim(insideTrimmed);
        if (insideTrimmed.find('+') != std::string::npos) {
            return OperandSubtype::OffsetMemory;
        }
        return OperandSubtype::Memory;
    }
    if (!operandText.empty() && operandText[0] == '#') {
        return OperandSubtype::Immediate;
    }
    {
        bool allDigits = !operandText.empty() &&
                         std::all_of(operandText.begin(), operandText.end(), ::isdigit);
        if (allDigits) {
            return OperandSubtype::Register;
        }
        static std::regex regRegex(R"(^(?:r)?\d+(?:\.[HL])?$)");
        if (std::regex_match(operandText, regRegex)) {
            return OperandSubtype::Register;
        }
    }
    if (!operandText.empty() && (std::isalpha(operandText[0]) || operandText[0] == '_')) {
        return OperandSubtype::LabelReference;
    }
    return OperandSubtype::Unknown;
}

// -----------------------------------------------
// First Pass: Collect Macros and Labels
// -----------------------------------------------
void Lexer::firstPass(const std::vector<std::string>& lines) {
    std::regex macroRegex(R"(^\s*\$(?:MACRO\s+)?([A-Za-z_]\w+)\s+(.+)$)");
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

        std::smatch m;
        if (std::regex_match(line, m, macroRegex)) {
            std::string macroName  = m[1];
            std::string macroValue = m[2];
            trim(macroName);
            trim(macroValue);
            macroTable[macroName] = macroValue;
            continue;
        }

        if (std::regex_match(line, m, labelRegex)) {
            std::string labelName = m[1];
            trim(labelName);
            labelTable[labelName] = i;
        }
    }
}

// -----------------------------------------------
// Second Pass: Tokenize
// -----------------------------------------------
std::vector<Token> Lexer::secondPass(const std::vector<std::string>& lines) {
    std::vector<Token> tokens;
    std::regex labelDef(R"(^\s*([A-Za-z_]\w*):\s*$)");
    std::regex instructionDef(R"(^[A-Za-z_]\w*$)");
    std::regex tokenRegex(R"((\[.*?\])|([^,\s]+))");

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string rawLine = lines[i];
        auto commentPos = rawLine.find(';');
        if (commentPos != std::string::npos) {
            rawLine.erase(commentPos);
        }
        trim(rawLine);
        if (rawLine.empty()) continue;
        if (!rawLine.empty() && rawLine[0] == '$') continue;

        std::string expanded = expandMacros(rawLine);
        trim(expanded);
        if (expanded.empty()) continue;

        std::smatch lm;
        if (std::regex_match(expanded, lm, labelDef)) {
            Token t;
            t.lexeme  = lm[1];
            t.type    = TokenType::Label;
            t.subtype = OperandSubtype::Unknown;
            t.data    = lm[1];
            tokens.push_back(t);
            continue;
        }

        std::sregex_iterator iter(expanded.begin(), expanded.end(), tokenRegex);
        std::sregex_iterator end;

        bool firstTokenOfLine = true;
        while (iter != end) {
            std::string thisToken;
            if ((*iter)[1].matched) {
                thisToken = (*iter)[1];
            } else {
                thisToken = (*iter)[2];
            }
            trim(thisToken);

            if (firstTokenOfLine) {
                if (std::regex_match(thisToken, instructionDef)) {
                    Token t;
                    t.lexeme  = thisToken;
                    t.type    = TokenType::Instruction;
                    t.subtype = OperandSubtype::Unknown;
                    t.data    = thisToken;
                    tokens.push_back(t);
                    firstTokenOfLine = false;
                } else {
                    Token t;
                    t.lexeme  = thisToken;
                    t.type    = TokenType::Operand;
                    t.subtype = parseOperandSubtype(thisToken);
                    t.data    = thisToken;
                    tokens.push_back(t);
                    firstTokenOfLine = false;
                }
            } else {
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