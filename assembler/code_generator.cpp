//
// Created by Dulat S on 1/27/25.
//

#include "code_generator.h"
#include "machine_description.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <unordered_map>

#include "assembler.h"

CodeGenerator::CodeGenerator() = default;
inline void trim(std::string &s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}
inline void store_big_endian(std::vector<uint8_t> &dest, uint64_t value) {
    int n = static_cast<int>(dest.size());
    for (int i = 0; i < n; i++) {
        dest[i] = static_cast<uint8_t>((value >> (8*(n - 1 - i))) & 0xFF);
    }
}
std::pair<std::string, std::string> split_register_suffix(const std::string &regToken) {
    auto dotPos = regToken.rfind('.');
    if (dotPos == std::string::npos) {
        return {regToken, ""}; // No suffix
    }

    std::string mainPart = regToken.substr(0, dotPos);
    std::string suffix = regToken.substr(dotPos + 1);

    // Validate suffix
    if (suffix != "L" && suffix != "H") {
        // Invalid suffix; treat entire string as mainPart
        return {regToken, ""};
    }

    return {mainPart, suffix};
}
// Assemble an instruction into object code
void CodeGenerator::assemble_instruction(const InstructionSpecifier *spec,
                                         const std::string &inst_name,
                                         const std::vector<Token> &operand_tokens,
                                         std::vector<uint8_t> &object_code) {
    // 1. Write out 'sp' and 'opcode'
    object_code.push_back(static_cast<uint8_t>(spec->sp));
    uint8_t opcode = get_opcode_for_instruction(inst_name.c_str());
    object_code.push_back(opcode);

    // 2. Build the placeholder map
    // Assumes that `build_placeholder_map` maps placeholders like "%rd" to tokens like "1.L", "5", etc.
    auto placeholder_map = build_placeholder_map(spec->syntax, operand_tokens);

    // 3. Get operand fields from machine encoding
    auto operand_fields = get_operand_lengths(inst_name, spec->sp);

    // Convert bit widths to byte widths
    for (auto &field : operand_fields) {
        field.second = static_cast<uint8_t>((field.second + 7) / 8);
    }

    // 4. Iterate over each operand field and encode accordingly
    for (const auto &[field_name, field_byte_width] : operand_fields) {
        if (field_name == "sp" || field_name == "opcode") {
            continue; // Already handled
        }

        // Initialize field bytes to zero
        std::vector<uint8_t> field_bytes(field_byte_width, 0);

        // Determine which token corresponds to this field
        std::string sub_field;
        std::string reg_suffix;
        const Token *chosen_token = find_token_for_field(field_name, placeholder_map, sub_field, reg_suffix);

        if (!chosen_token) {
            std::cerr << "ERROR: No matching token for field '" << field_name << "'\n";
            continue; // Skip encoding this field
        }

        uint64_t value_to_store = 0; // Value to be encoded

        // Encode based on operand subtype and field specifics
        switch (chosen_token->subtype) {
            case OperandSubtype::Immediate: {
                // Immediate values like "#8"
                std::string imm_str = chosen_token->data;
                if (!imm_str.empty() && imm_str[0] == '#') {
                    imm_str.erase(0, 1); // Remove '#'
                }
                try {
                    value_to_store = std::stoll(imm_str, nullptr, 0);
                } catch (const std::invalid_argument &) {
                    std::cerr << "ERROR: Invalid immediate value '" << imm_str << "'\n";
                    continue;
                }
                break;
            }
            case OperandSubtype::Register: {
                // Register tokens like "1.L", "5", "3.H"
                // Split into main part and suffix
                auto [mainPart, suffix] = split_register_suffix(chosen_token->data);

                // Parse register number
                int reg_num = 0;
                try {
                    reg_num = std::stoi(mainPart, nullptr, 0);
                } catch (const std::invalid_argument &) {
                    std::cerr << "ERROR: Invalid register number '" << mainPart << "'\n";
                    continue;
                }

                // Initialize register field with register number (6 bits)
                uint8_t reg_field = static_cast<uint8_t>(reg_num & 0x3F); // bits 5..0

                // Handle suffix
                if (!suffix.empty()) {
                    if (suffix == "H") {
                        reg_field |= 0x80; // Set bit7
                    }
                    else if (suffix == "L") {
                        reg_field |= 0x40; // Set bit6
                    }
                    // Ensure that both bits are not set simultaneously
                    if ((reg_field & 0xC0) == 0xC0) {
                        std::cerr << "ERROR: Register '" << chosen_token->data
                                  << "' cannot have both .L and .H suffixes.\n";
                        continue;
                    }
                }

                value_to_store = reg_field;
                break;
            }
            case OperandSubtype::Memory: {
                // Memory operands like "[5 + #8]" or "[#0x100]"
                // Extract the address value
                std::string inside = chosen_token->data;
                if (!inside.empty() && inside.front() == '[') inside.erase(0, 1);
                if (!inside.empty() && inside.back() == ']') inside.pop_back();
                trim(inside);

                // Remove leading '#' if present
                if (!inside.empty() && inside[0] == '#') {
                    inside.erase(0, 1);
                }

                try {
                    value_to_store = std::stoll(inside, nullptr, 0);
                } catch (const std::invalid_argument &) {
                    std::cerr << "ERROR: Invalid memory address '" << inside << "'\n";
                    continue;
                }
                break;
            }
            case OperandSubtype::OffsetMemory: {
                // OffsetMemory operands like "[5 + #8]"
                auto [base_val, offset_val] = parse_offset_memory_subfields(chosen_token->data);

                if (sub_field == "baseReg") {
                    // Base register (no suffixes)
                    if (base_val < 0 || base_val > 63) { // Assuming 6-bit register encoding
                        std::cerr << "ERROR: Base register number '" << base_val
                                  << "' out of range (0-63).\n";
                        continue;
                    }
                    value_to_store = static_cast<uint8_t>(base_val & 0x3F); // bits5..0
                }
                else if (sub_field == "offset") {
                    value_to_store = static_cast<uint64_t>(offset_val);
                }
                else {
                    std::cerr << "ERROR: Unknown subfield '" << sub_field
                              << "' for OffsetMemory.\n";
                    continue;
                }
                break;
            }
            default:
                std::cerr << "ERROR: Unhandled operand subtype for token '"
                          << chosen_token->data << "'\n";
                continue;
        }

        // Encode the value into field_bytes in big-endian order
        store_big_endian(field_bytes, value_to_store);

        // Append the encoded bytes to object_code
        object_code.insert(object_code.end(), field_bytes.begin(), field_bytes.end());
    }
}

// Get operand lengths from machine encoding definition
std::vector<std::pair<std::string, uint8_t> >
CodeGenerator::get_operand_lengths(const std::string &inst_name, uint8_t sp) {
    const char *encoding = get_encoding_for_instruction(inst_name.c_str(), sp);
    if (!encoding) {
        // Handle null or unknown
        return {};
    }

    std::string enc_str(encoding);
    std::istringstream iss(enc_str);
    std::string token_str;

    // This will store (field_name, bit_width) in the order discovered
    std::vector<std::pair<std::string, uint8_t> > fields;

    while (iss >> token_str) {
        // Strip surrounding brackets if present, e.g. [sp(8)] -> sp(8)
        if (!token_str.empty() && token_str.front() == '[') token_str.erase(0, 1);
        if (!token_str.empty() && token_str.back() == ']') token_str.pop_back();

        // The token should be in the form fieldName(bitWidth)
        auto paren_open = token_str.find('(');
        auto paren_close = token_str.find(')');
        if (paren_open == std::string::npos || paren_close == std::string::npos ||
            paren_close < paren_open) {
            // Malformed token; skip or throw error
            continue;
        }

        std::string field_name = token_str.substr(0, paren_open);
        std::string width_str = token_str.substr(paren_open + 1, paren_close - paren_open - 1);
        auto bit_width = static_cast<uint8_t>(std::stoi(width_str));

        // Skip fields you don't want to treat as operands:
        // typically "sp", "opcode", or any other meta-fields.
        if (field_name == "sp" || field_name == "opcode") {
            continue;
        }

        // Push the pair (field_name, bit_width) into our vector
        fields.emplace_back(field_name, bit_width);
    }

    return fields;
}

// Build a map from placeholder to token
std::unordered_map<std::string, Token>
CodeGenerator::build_placeholder_map(const std::string &syntax_str,
                                     const std::vector<Token> &operand_tokens) {
    std::unordered_map<std::string, Token> placeholder_map;

    // 1) Extract just the operand-part of the syntax (ignore the mnemonic)
    //    e.g. from "mov %rd, #%immediate" => "%rd, #%immediate"
    auto first_space = syntax_str.find(' ');
    if (first_space == std::string::npos) {
        // Means there's no space => possibly an instruction with no operands
        // If operand_tokens is empty, we return an empty map.
        return placeholder_map;
    }
    std::string operand_part = syntax_str.substr(first_space + 1);

    // 2) Split that operand_part on commas to find individual placeholders
    //    e.g. "%rd, #%immediate" => ["%rd", "#%immediate"]
    std::vector<std::string> placeholders;
    std::istringstream iss(operand_part);
    std::string tmp_str;
    while (std::getline(iss, tmp_str, ',')) {
        // Trim whitespace
        tmp_str.erase(tmp_str.begin(), std::find_if(tmp_str.begin(), tmp_str.end(),
                    [](unsigned char ch) { return !std::isspace(ch); }));
        tmp_str.erase(std::find_if(tmp_str.rbegin(), tmp_str.rend(),
                    [](unsigned char ch) { return !std::isspace(ch); }).base(), tmp_str.end());
        if (!tmp_str.empty()) {
            placeholders.push_back(tmp_str);
        }
    }

    // 3) Basic sanity check: number of placeholders == number of operand_tokens
    if (placeholders.size() != operand_tokens.size()) {
        std::cerr << "Mismatch between placeholder count and operand token count!\n";
        return placeholder_map;
    }

    // 4) Assign each placeholder to the corresponding Token in order.
    for (size_t i = 0; i < placeholders.size(); i++) {
        placeholder_map[placeholders[i]] = operand_tokens[i];
    }

    return placeholder_map;
}

// Helper: parse something like "[2 + #8]" => (base_reg=2, offset=8)
std::pair<int, int> CodeGenerator::parse_offset_memory_subfields(const std::string &token_data) {
    // If we already parsed this exact token string, use the cache:
    auto cache_it = offset_memory_cache.find(token_data);
    if (cache_it != offset_memory_cache.end()) {
        return cache_it->second;
    }

    // token_data is e.g. "[2 + #8]" or "[#0x100 + #16]" etc.
    // Remove outer brackets:
    std::string content = token_data.substr(1, token_data.size() - 2);

    // Split on '+':
    auto plus_pos = content.find('+');
    if (plus_pos == std::string::npos) {
        throw std::runtime_error("OffsetMemory operand missing '+': " + token_data);
    }
    std::string base_str = content.substr(0, plus_pos);
    std::string offset_str = content.substr(plus_pos + 1);

    // Trim whitespace:
    auto trim = [](std::string &s) {
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    };
    trim(base_str);
    trim(offset_str);

    // base_str might be "2" or "3", or possibly "#0x10" if your assembler design allows that.
    // offset_str might be "#8", "#0x10", etc.
    int base_val = 0;
    if (!base_str.empty() && base_str[0] == '#') {
        base_val = std::stoi(base_str.substr(1), nullptr, 0);
    }
    else {
        base_val = std::stoi(base_str, nullptr, 0);
    }

    int offset_val = 0;
    if (!offset_str.empty() && offset_str[0] == '#') {
        offset_val = std::stoi(offset_str.substr(1), nullptr, 0);
    }
    else {
        offset_val = std::stoi(offset_str, nullptr, 0);
    }

    offset_memory_cache[token_data] = {base_val, offset_val};
    return {base_val, offset_val};
}
const Token* CodeGenerator::find_token_for_field(const std::string &field_name,
                                  const std::unordered_map<std::string, Token> &placeholder_map,
                                  std::string &subFieldOut,
                                  std::string &regSuffixOut) {
    subFieldOut.clear();
    regSuffixOut.clear();

    // Determine if the field is a register that can have suffixes
    bool is_register_field =
        (field_name == "rd" || field_name == "rd1" ||
         field_name == "rn" || field_name == "rn1" ||
         field_name == "rm" || field_name == "rs"); // Extend as needed

    if (is_register_field) {
        // 1. Construct the placeholder key with '%'
        std::string directKey = "%" + field_name; // e.g., "%rd"

        // 2. Attempt exact match
        auto it = placeholder_map.find(directKey);
        if (it != placeholder_map.end()) {
            // Found exact match; no suffix
            return &it->second;
        }

        // 3. Attempt to find a token with suffix (e.g., "%rd.L", "%rd.H")
        for (const auto &kv : placeholder_map) {
            const std::string &ph = kv.first; // e.g., "%rd.L"
            if (ph.find(directKey + ".") == 0) { // Starts with "%rd."
                auto [mainPart, suffix] = split_register_suffix(ph.substr(1)); // Remove '%'
                if (mainPart == field_name) {
                    regSuffixOut = suffix; // "L" or "H"
                    return &kv.second;
                }
            }
        }

        // 4. If still not found, check if it's part of an OffsetMemory operand
        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::OffsetMemory) {
                subFieldOut = "baseReg";
                return &kv.second;
            }
        }

        // Not found
        return nullptr;
    }
    else if (field_name == "offset") {
        // Offset field from OffsetMemory operand
        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::OffsetMemory) {
                subFieldOut = "offset";
                return &kv.second;
            }
        }
    }
    else if (field_name == "immediate" || field_name == "imm" || field_name == "operand2") {
        // Immediate operand
        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::Immediate) {
                return &kv.second;
            }
        }
    }
    else if (field_name == "normAddressing") {
        // Memory operand
        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::Memory) {
                return &kv.second;
            }
        }
    }

    // No match found
    return nullptr;
}