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
// Assemble an instruction into object code
void CodeGenerator::assemble_instruction(const InstructionSpecifier *spec,
                                         const std::string &inst_name,
                                         const std::vector<Token> &operand_tokens,
                                         std::vector<uint8_t> &object_code)
{
    // 1) Write out 'sp' (sub-opcode/secondary prefix) and 'opcode'
    object_code.push_back(static_cast<uint8_t>(spec->sp));
    uint8_t opcode = get_opcode_for_instruction(inst_name.c_str());
    object_code.push_back(opcode);

    // 2) Build the placeholder map (maps something like "%rd" or "[%rn + #%offset]" -> Token)
    auto placeholder_map = build_placeholder_map(spec->syntax, operand_tokens);

    // 3) Get the operand fields from the machine encoding
    //    e.g. for spec->encoding == "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(32)]"
    //    we might get vector of: {("rd",8), ("rn",8), ("offset",32)}
    auto operand_fields = get_operand_lengths(inst_name, spec->sp);

    // Convert from bits to bytes
    for (auto &field : operand_fields) {
        field.second = static_cast<uint8_t>((field.second + 7) / 8);
    }

    // 4) For each field in the encoding (in order), fill the bytes accordingly
    for (auto &[field_name, field_byte_width] : operand_fields)
    {
        // We skip sp/opcode if they appear in the encoding string, but typically we also
        // filtered them out in get_operand_lengths(). Just in case:
        if (field_name == "sp" || field_name == "opcode") {
            // Already handled at the start
            continue;
        }

        // Allocate a zeroed buffer for that field
        std::vector<uint8_t> field_bytes(field_byte_width, 0);

        // We figure out which token and subfield we need:
        std::string sub_field;
        const Token *chosen_token = find_token_for_field(field_name, placeholder_map, sub_field);

        if (!chosen_token) {
            std::cerr << "ERROR: Couldn't find a matching token for field '"
                      << field_name << "'\n";
            // You might want to throw an exception or handle more gracefully
            continue;
        }

        // Now encode that token into field_bytes
        switch (chosen_token->subtype)
        {
        case OperandSubtype::Immediate: {
            // e.g. data = "#123" or "#0xFF"
            // strip the '#' and parse:
            std::string imm_str = chosen_token->data;
            if (!imm_str.empty() && imm_str[0] == '#') {
                imm_str.erase(0, 1); // remove '#'
            }
            int value = std::stoi(imm_str, nullptr, 0);
            for (int b = 0; b < (int)field_byte_width; ++b) {
                // store big-endian or little-endian? Typically little-endian is usual:
                field_bytes[b] = (value >> (8*b)) & 0xFF;
            }
            break;
        }
        case OperandSubtype::Register: {
            // e.g. data = "%3", "3", "%2.L", etc.
            // strip leading '%' if present, also remove .L/.H
            std::string reg_str = chosen_token->data;
            if (!reg_str.empty() && reg_str[0] == '%') {
                reg_str.erase(0, 1);
            }
            // remove .L or .H if present
            if (auto pos = reg_str.find(".L"); pos != std::string::npos) {
                reg_str.erase(pos);
            } else if (auto pos = reg_str.find(".H"); pos != std::string::npos) {
                reg_str.erase(pos);
            }
            int reg_num = std::stoi(reg_str, nullptr, 0);
            for (int b = 0; b < (int)field_byte_width; ++b) {
                field_bytes[b] = (reg_num >> (8*b)) & 0xFF;
            }
            break;
        }
        case OperandSubtype::Memory: {
            // e.g. data = "[#0x100]" or "[0x200]"
            // remove outer brackets:
            std::string inside = chosen_token->data;
            if (!inside.empty() && inside.front() == '[') inside.erase(0,1);
            if (!inside.empty() && inside.back() == ']') inside.pop_back();
            trim(inside);
            // Possibly inside is "#0x100" => remove '#'
            if (!inside.empty() && inside[0] == '#') {
                inside.erase(0,1);
            }
            int mem_val = std::stoi(inside, nullptr, 0);
            // store in little-endian
            for (int b = 0; b < (int)field_byte_width; ++b) {
                field_bytes[b] = (mem_val >> (8*b)) & 0xFF;
            }
            break;
        }
        case OperandSubtype::OffsetMemory: {
            // e.g. data = "[2 + #8]" or "[%3 + #0x10]" or "[#0x100 + #16]"
            auto [base_val, offset_val] = parse_offset_memory_subfields(chosen_token->data);

            // Decide which subfield we want. If sub_field == "baseReg", we store base_val,
            // else if sub_field == "offset", we store offset_val.
            int value_to_store = 0;
            if (sub_field == "baseReg") {
                value_to_store = base_val;
            }
            else if (sub_field == "offset") {
                value_to_store = offset_val;
            }
            // else you might want to complain if neither

            for (int b = 0; b < (int)field_byte_width; ++b) {
                field_bytes[b] = (value_to_store >> (8*b)) & 0xFF;
            }
            break;
        }
        default:
            std::cerr << "ERROR: Unhandled operand subtype for token '"
                      << chosen_token->data << "'\n";
            break;
        }

        // Finally, append these field bytes to the object code
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
                                  std::string &sub_field_out)
{
    // Reset sub_field_out each time:
    sub_field_out.clear();

    // 1) Try direct placeholder name => e.g. field_name "rd" -> placeholder "%rd"
    //                                     field_name "rd1" -> placeholder "%rd1"
    //                                     field_name "rn" -> placeholder "%rn"
    //                                     field_name "rn1" -> placeholder "%rn1"
    // We do this only if field_name starts with "r".
    if (field_name == "rd" || field_name == "rn" ||
        field_name == "rd1" || field_name == "rn1")
    {
        // Construct the placeholder key: e.g. "%rd"
        std::string placeholder_key = "%" + field_name;  // "rd" -> "%rd", "rn1" -> "%rn1", etc.

        auto it = placeholder_map.find(placeholder_key);
        if (it != placeholder_map.end()) {
            // Found a direct match. We'll return it:
            return &it->second;
        }

        // If not found, it might be part of an offsetMemory token, e.g. "[%rn + #%offset]"
        // In that case, we want the "baseReg" subfield.
        // But that means we must see if there's exactly one offsetMemory token in the map:
        for (auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::OffsetMemory) {
                // We'll assume there's only one offsetMemory placeholder in this instruction
                // that is supposed to fill "rn" or "rn1".
                sub_field_out = "baseReg";
                return &kv.second;
            }
        }
    }
    else if (field_name == "offset") {
        // This usually comes from an offsetMemory placeholder
        for (auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::OffsetMemory) {
                sub_field_out = "offset";
                return &kv.second;
            }
        }
        // Possibly you might fail here if there's no offsetMemory token.
    }
    else if (field_name == "immediate" ||
             field_name == "imm" ||
             field_name == "operand2")
    {
        // Look for a placeholder whose subtype == Immediate
        for (auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::Immediate) {
                return &kv.second;
            }
        }
    }
    else if (field_name == "normAddressing") {
        // Look for a placeholder with subtype == Memory
        for (auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::Memory) {
                return &kv.second;
            }
        }
    }

    // If we get here, we couldn't find anything for field_name
    return nullptr;
}