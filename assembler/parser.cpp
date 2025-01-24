//
// Created by gitpod on 2/7/24.
//

#include <limits>
#include <algorithm>
#include "parser.h"
#include "assembler.h"
#include "machine_description.h"
#include "sstream"

void Parser::parse() {
    currentTokenIndex = 0;

    while (currentTokenIndex < tokens.size()) {
        const Token &t = tokens[currentTokenIndex];

        if (t.type == TokenType::Label) {
            currentTokenIndex++;
        } else if (t.type == TokenType::Instruction) {
            try {
                // Parse the instruction and get the mnemonic and specifier syntax.
                parse_instruction();
            } catch (const std::exception &e) {
                std::cerr << e.what() << "\n";
            }
        } else {
            std::cerr << "Unexpected token: " << t.data
                    << " at index " << currentTokenIndex << "\n";
            currentTokenIndex++;
        }
    }
}


void Parser::parse_instruction() {
    // 1) Current token => instruction
    const Token &inst_token = tokens[currentTokenIndex];
    std::string inst_name = inst_token.data; // e.g. "mov", "add", "hlt"
    currentTokenIndex++;

    // 2) Gather operand tokens until next instruction or label
    std::vector<Token> operand_tokens;
    while (currentTokenIndex < tokens.size()) {
        const Token &lookahead = tokens[currentTokenIndex];
        if (lookahead.type == TokenType::Instruction ||
            lookahead.type == TokenType::Label) {
            break; // next instruction or label => stop collecting
        }
        operand_tokens.push_back(lookahead);
        currentTokenIndex++;
    }

    // 3) Find the instruction format
    const InstructionFormat *fmt = find_instruction_format(inst_name.c_str());
    if (!fmt) {
        throw std::runtime_error("Unknown instruction: " + inst_name);
    }

    // We'll attempt to match operandTokens against each specifier's syntax.
    // For example, a specifier might have syntax = "mov %rd, #%immediate".
    const InstructionSpecifier *chosenSpec = nullptr;

    for (size_t i = 0; i < fmt->num_specifiers; ++i) {
        const InstructionSpecifier &spec = fmt->specifiers[i];
        if (match_operands_against_syntax(operand_tokens, spec.syntax)) {
            chosenSpec = &spec;
            break;
        }
    }

    if (!chosenSpec) {
        // None of the specifiers' syntax matched
        throw std::runtime_error(
            "No matching syntax for '" + inst_name + "' with the given operands."
        );
    }

    // Now 'chosenSpec' is the specifier that matched
    // ...
    // next, extract operands from the tokens and encode the instruction
    assemble_instruction(chosenSpec, inst_name, operand_tokens);
}

void Parser::assemble_instruction(const InstructionSpecifier *spec,
                                  const std::string &inst_name,
                                  const std::vector<Token> &operand_tokens)
{
    //
    // 1) Write out 'sp' and 'opcode' as before
    //
    object_code.push_back(static_cast<uint8_t>(spec->sp));
    uint8_t opcode = get_opcode_for_instruction(inst_name.c_str());
    object_code.push_back(opcode);

    //
    // 2) Build a map from "syntax placeholder" -> "actual Token"
    //
    //    e.g. if spec->syntax is "mov %rd, #%immediate"
    //    and operand_tokens are [Token("%3", Register), Token("#12", Immediate)],
    //    then placeholderMap["%rd"] = Token("%3", Register)
    //         placeholderMap["#%immediate"] = Token("#12", Immediate)
    //
    auto placeholderMap = build_placeholder_map(spec->syntax, operand_tokens);

    //
    // 3) Retrieve the (fieldName, bitWidth) pairs in *the order* they appear
    //    in the machine-encoding definition
    //
    auto operandFields = get_operand_lengths(inst_name, spec->sp);
    // Convert bitWidth to byteWidth in place
    for (auto &p : operandFields) {
        p.second = (uint8_t)((p.second + 7) / 8);
    }

    //
    // 4) For each field in 'operandFields' (which is in the order from the encoding
    //    string), find the correct token and encode it.
    //
    //    We'll do a small helper function to map fieldName -> placeholderKey.
    //
    auto fieldNameToPlaceholder = [&](const std::string &fn) -> std::string {
        // You could do a more advanced mapping. For example, if
        //  fieldName == "destReg" then the placeholder might be "%rd" or "rd".
        //  We'll do a simple approach:
        //    1) Try an exact match in placeholderMap
        //    2) If not found, try adding '%' or '#' or removing them
        //    3) Return whichever is found or empty if none

        // 1) If there's an exact match
        if (placeholderMap.contains(fn)) {
            return fn;
        }

        // 2) Try with a '%' if not already
        {
            std::string alt = "%" + fn;  // e.g. fieldName "rd" => "%rd"
            if (placeholderMap.find(alt) != placeholderMap.end()) {
                return alt;
            }
        }
        // 3) Or a '#' in front if it looks immediate
        {
            std::string alt = "#%" + fn; // e.g. "immediate" => "#%immediate"
            if (placeholderMap.find(alt) != placeholderMap.end()) {
                return alt;
            }
        }
        // Could add more logic (like searching among placeholderMap keys)...

        // If we fail, return something empty or do a direct pass-through
        return std::string{};
    };

    // 5) Encode each field in the exact order from the encoding
    for (auto &[fieldName, fieldByteWidth] : operandFields)
    {
        // (A) Identify which placeholder is responsible for this fieldName
        std::string placeholder = fieldNameToPlaceholder(fieldName);
        if (placeholder.empty()) {
            // If nothing found, handle error or continue
            std::cerr << "No placeholder found for field '" << fieldName << "'\n";
            continue;
        }

        // (B) Retrieve the matching token
        auto itToken = placeholderMap.find(placeholder);
        if (itToken == placeholderMap.end()) {
            // no matching token => error
            std::cerr << "No token for placeholder '" << placeholder << "'\n";
            continue;
        }
        const Token &tok = itToken->second;

        // (C) Prepare a buffer to encode this field
        std::vector<uint8_t> fieldBytes(fieldByteWidth, 0);

        // (D) Encode based on token subtype
        if (tok.subtype == OperandSubtype::Immediate) {
            // e.g. "#123"
            int value = std::stoi(tok.data.substr(1)); // remove '#'
            for (int b = 0; b < (int)fieldByteWidth; ++b) {
                fieldBytes[b] = (value >> (8 * (fieldByteWidth - 1 - b))) & 0xFF;
            }
        }
        else if (tok.subtype == OperandSubtype::Register) {
            // e.g. "3", "3.H", "3.L"
            std::string reg = tok.data;
            if (auto pos = reg.find(".H"); pos != std::string::npos)
                reg = reg.substr(0, pos);
            else if (auto pos = reg.find(".L"); pos != std::string::npos)
                reg = reg.substr(0, pos);

            int reg_num = std::stoi(reg);
            fieldBytes[0] = static_cast<uint8_t>(reg_num & 0xFF);
        }
        else if (tok.subtype == OperandSubtype::Memory) {
            // e.g. "[#0xFF]"
            std::string content = tok.data.substr(1, tok.data.size() - 2); // remove [ ]
            int memVal = 0;
            if (!content.empty() && content[0] == '#') {
                memVal = std::stoi(content.substr(1), nullptr, 0);
            } else {
                memVal = std::stoi(content);
            }
            for (int b = 0; b < (int)fieldByteWidth; ++b) {
                fieldBytes[b] = (memVal >> (8 * (fieldByteWidth - 1 - b))) & 0xFF;
            }
        }
        else if (tok.subtype == OperandSubtype::OffsetMemory) {
            // e.g. "[2 + #8]".
            // Some architectures put "baseReg" and "offset" in separate fields.
            // This single token might fill multiple encoding fields.
            // For *truly* non-sequential usage, you'd likely find separate
            // fields named "baseReg" and "offset" in operandFields.
            //
            // But here we see only "fieldByteWidth" for *one* fieldName.
            // If your architecture lumps baseReg+offset into *one* field
            // in the encoding, you can parse and pack them here.
            //
            // For demonstration, let's assume the entire "[base + #offset]"
            // is one field the same size:
            std::string content = tok.data.substr(1, tok.data.size() - 2);
            auto plusPos = content.find('+');
            std::string baseStr   = content.substr(0, plusPos);
            std::string offsetStr = content.substr(plusPos + 1);

            // trim
            auto trim = [](std::string &s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                    [](unsigned char c){ return !std::isspace(c); }));
                s.erase(std::find_if(s.rbegin(), s.rend(),
                    [](unsigned char c){ return !std::isspace(c); }).base(), s.end());
            };
            trim(baseStr);
            trim(offsetStr);

            int baseVal  = std::stoi(baseStr);
            int offsetVal = 0;
            if (!offsetStr.empty() && offsetStr[0] == '#') {
                offsetVal = std::stoi(offsetStr.substr(1));
            }
            // Combine them however your encoding expects.
            // For example, if the single field is 16 bits: high 8 bits = baseReg, low 8 bits = offset
            // This is just an example:
            uint16_t combined = static_cast<uint16_t>((baseVal & 0xFF) << 8) | (offsetVal & 0xFF);
            for (int b = 0; b < (int)fieldByteWidth; ++b) {
                fieldBytes[b] = (combined >> (8 * (fieldByteWidth - 1 - b))) & 0xFF;
            }
        }
        else {
            // Unknown subtype, skip or handle error
            continue;
        }

        // (E) Append to object code
        object_code.insert(object_code.end(), fieldBytes.begin(), fieldBytes.end());
    }
}

bool Parser::match_operands_against_syntax(const std::vector<Token> &operand_tokens,
                                           const std::string &syntax_str) {
    // Example syntaxStr: "mov %rd, #%immediate"
    // We want to split on commas, ignoring the instruction mnemonic at the front if it exists.
    // Then we compare each piece: e.g. "%rd" <-> register operand, "#%immediate" <-> immediate operand, etc.

    // 1) Separate the instruction mnemonic (e.g. "mov") from the operand placeholders
    //    We'll assume the first token in syntaxStr is always the mnemonic.
    //    So let's isolate the substring after the first space:

    auto firstSpace = syntax_str.find(' ');
    if (firstSpace == std::string::npos) {
        // means there's no space => probably no operands
        // so if we have no operand tokens, match is true; else false
        return operand_tokens.empty();
    }

    // substring after the mnemonic
    // e.g. from "mov %rd, #%immediate" => "%rd, #%immediate"
    std::string operandPart = syntax_str.substr(firstSpace + 1);

    // 2) Split that on commas:
    std::vector<std::string> syntaxOperands; {
        std::istringstream iss(operandPart);
        std::string temp;
        while (std::getline(iss, temp, ',')) {
            // trim leading/trailing space
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.front()))) temp.erase(temp.begin());
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.back()))) temp.pop_back();
            syntaxOperands.push_back(temp);
        }
    }

    // If the number of syntax operands doesn't match the number of actual operand tokens, no match
    if (syntaxOperands.size() != operand_tokens.size()) {
        return false;
    }

    // 3) Compare each syntax operand placeholder against the token’s subtype
    for (size_t i = 0; i < syntaxOperands.size(); i++) {
        const std::string &placeholder = syntaxOperands[i];
        const Token &actualToken = operand_tokens[i];

        // e.g. placeholder = "%rd", "#%immediate", "[%mem]", ...
        // check if it’s a register placeholder, immediate placeholder, memory, etc.
        if (!placeholder_matches_token(placeholder, actualToken)) {
            return false; // mismatch
        }
    }

    // If all placeholders matched, return true
    return true;
}

bool Parser::placeholder_matches_token(const std::string &placeholder, const Token &tok) {
    // Handle Offset Memory Addressing
    if (placeholder.find("[%rn + #%offset]") != std::string::npos) {
        return (tok.subtype == OperandSubtype::OffsetMemory);
    }
    // Handle Register Placeholders
    else if (placeholder.find("%rd") != std::string::npos ||
             placeholder.find("%rn") != std::string::npos ||
             placeholder.find("%rd1") != std::string::npos ||
             placeholder.find("%rn1") != std::string::npos) {
        // Ensure the token is a Register
        if (tok.subtype != OperandSubtype::Register) {
            return false;
        }

        // Check for ".H" or ".L" in the placeholder
        bool expectsHigh = placeholder.find(".H") != std::string::npos;
        bool expectsLow = placeholder.find(".L") != std::string::npos;

        // Check for ".H" or ".L" in the token's data
        bool hasHigh = tok.data.find(".H") != std::string::npos;
        bool hasLow = tok.data.find(".L") != std::string::npos;

        if (expectsHigh) {
            return hasHigh;
        } else if (expectsLow) {
            return hasLow;
        } else {
            // Placeholder does not specify ".H" or ".L"
            // Ensure token's data does not contain either
            return !hasHigh && !hasLow;
        }
    }
    // Handle Immediate Operands
    else if (placeholder.find("#%immediate") != std::string::npos) {
        return (tok.subtype == OperandSubtype::Immediate);
    }
    // Handle Normal Memory Addressing
    else if (placeholder.find("[%normAddressing]") != std::string::npos) {
        return (tok.subtype == OperandSubtype::Memory);
    }
    // If no known placeholder matched, return false
    return false;
}

std::vector<std::pair<std::string, uint8_t>>
Parser::get_operand_lengths(const std::string &inst_name, uint8_t sp) {
    const char *encoding = get_encoding_for_instruction(inst_name.c_str(), sp);
    if (!encoding) {
        // Handle null or unknown
        return {};
    }

    std::string enc_str(encoding);
    std::istringstream iss(enc_str);
    std::string token;

    // This will store (fieldName, bitWidth) in the order discovered
    std::vector<std::pair<std::string, uint8_t>> fields;

    while (iss >> token) {
        // Strip surrounding brackets if present, e.g. [sp(8)] -> sp(8)
        if (!token.empty() && token.front() == '[') token.erase(0, 1);
        if (!token.empty() && token.back() == ']')   token.pop_back();

        // The token should be in the form fieldName(bitWidth)
        auto parenOpen  = token.find('(');
        auto parenClose = token.find(')');
        if (parenOpen == std::string::npos || parenClose == std::string::npos ||
            parenClose < parenOpen)
        {
            // Malformed token; skip or throw error
            continue;
        }

        std::string field_name = token.substr(0, parenOpen);
        std::string widthStr  = token.substr(parenOpen + 1, parenClose - parenOpen - 1);
        auto bit_width      = static_cast<uint8_t>(std::stoi(widthStr));

        // Skip fields you don't want to treat as operands:
        // typically "sp", "opcode", or any other meta-fields.
        if (field_name == "sp" || field_name == "opcode") {
            continue;
        }

        // Push the pair (fieldName, bitWidth) into our vector
        fields.emplace_back(field_name, bit_width);
    }

    return fields;
}
std::unordered_map<std::string, Token>
Parser::build_placeholder_map(const std::string &syntax_str,
                      const std::vector<Token> &operand_tokens)
{
    std::unordered_map<std::string, Token> placeholderMap;

    // 1) Extract just the operand-part of the syntax (ignore the mnemonic)
    //    e.g. from "mov %rd, #%immediate" => "%rd, #%immediate"
    auto firstSpace = syntax_str.find(' ');
    if (firstSpace == std::string::npos) {
        // Means there's no space => possibly an instruction with no operands
        // If operand_tokens is empty, we return an empty map.
        return placeholderMap;
    }
    std::string operandPart = syntax_str.substr(firstSpace + 1);

    // 2) Split that operandPart on commas to find individual placeholders
    //    e.g. "%rd, #%immediate" => ["%rd", "#%immediate"]
    std::vector<std::string> placeholders;
    {
        std::istringstream iss(operandPart);
        std::string tmp;
        while (std::getline(iss, tmp, ',')) {
            // trim whitespace
            while (!tmp.empty() && std::isspace((unsigned char)tmp.front())) {
                tmp.erase(tmp.begin());
            }
            while (!tmp.empty() && std::isspace((unsigned char)tmp.back())) {
                tmp.pop_back();
            }
            if (!tmp.empty()) {
                placeholders.push_back(tmp);
            }
        }
    }

    // 3) Basic sanity check: number of placeholders == number of operand_tokens
    if (placeholders.size() != operand_tokens.size()) {
        // In a fully robust assembler, you'd handle this mismatch gracefully
        // or throw an exception. We'll just print an error for demonstration.
        std::cerr << "Mismatch between placeholder count and operand token count!\n";
        return placeholderMap;
    }

    // 4) Assign each placeholder to the corresponding Token in order.
    //    Because we've already validated the tokens with match_operands_against_syntax(...),
    //    we know operand_tokens[i] definitely matches placeholders[i].
    for (size_t i = 0; i < placeholders.size(); i++) {
        // Optionally, you could do an additional safety check:
        //    if (!placeholder_matches_token(placeholders[i], operand_tokens[i])) {
        //        // throw or log error
        //    }

        placeholderMap[ placeholders[i] ] = operand_tokens[i];
    }

    return placeholderMap;
}