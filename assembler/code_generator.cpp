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

// Remove extra spaces from a string.
inline void trim(std::string &s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
        s.pop_back();
}

// Save a number into a byte vector in big-endian order.
inline void store_big_endian(std::vector<uint8_t> &dest, uint64_t value) {
    int n = static_cast<int>(dest.size());
    for (int i = 0; i < n; i++) {
        dest[i] = static_cast<uint8_t>((value >> (8 * (n - 1 - i))) & 0xFF);
    }
}

// Break a register token into its number and suffix.
// If no valid suffix, return the token with an empty suffix.
std::pair<std::string, std::string> split_register_suffix(const std::string &regToken) {
    auto dotPos = regToken.rfind('.');
    if (dotPos == std::string::npos)
        return {regToken, ""};

    std::string mainPart = regToken.substr(0, dotPos);
    std::string suffix = regToken.substr(dotPos + 1);

    if (suffix != "L" && suffix != "H")
        return {regToken, ""};

    return {mainPart, suffix};
}

// Turn an instruction into object code.
void CodeGenerator::assemble_instruction(const InstructionSpecifier *spec,
                                           const std::string &inst_name,
                                           const std::vector<Token> &operand_tokens,
                                           std::vector<uint8_t> &object_code) {
    // Write the sp and opcode.
    object_code.push_back(static_cast<uint8_t>(spec->sp));
    uint8_t opcode = get_opcode_for_instruction(inst_name.c_str());
    object_code.push_back(opcode);

    // Map placeholders from the syntax to tokens.
    auto placeholder_map = build_placeholder_map(spec->syntax, operand_tokens);

    // Get operand fields and convert bit widths to byte widths.
    auto operand_fields = get_operand_lengths(inst_name, spec->sp);
    for (auto &field : operand_fields)
        field.second = static_cast<uint8_t>((field.second + 7) / 8);

    // Process each operand field.
    for (const auto &[field_name, field_byte_width] : operand_fields) {
        if (field_name == "sp" || field_name == "opcode")
            continue;

        std::vector<uint8_t> field_bytes(field_byte_width, 0);
        std::string sub_field, reg_suffix;
        const Token *chosen_token = find_token_for_field(field_name, placeholder_map, sub_field, reg_suffix);

        if (!chosen_token) {
            std::cerr << "ERROR: No matching token for field '" << field_name << "'\n";
            continue;
        }

        uint64_t value_to_store = 0;

        switch (chosen_token->subtype) {
            case OperandSubtype::Immediate: {
                std::string imm_str = chosen_token->data;
                if (!imm_str.empty() && imm_str[0] == '#')
                    imm_str.erase(0, 1);
                try {
                    value_to_store = std::stoll(imm_str, nullptr, 0);
                } catch (const std::invalid_argument &) {
                    std::cerr << "ERROR: Invalid immediate value '" << imm_str << "'\n";
                    continue;
                }
                break;
            }
            case OperandSubtype::Register: {
                auto [mainPart, suffix] = split_register_suffix(chosen_token->data);
                int reg_num = 0;
                try {
                    reg_num = std::stoi(mainPart, nullptr, 0);
                } catch (const std::invalid_argument &) {
                    std::cerr << "ERROR: Invalid register number '" << mainPart << "'\n";
                    continue;
                }

                uint8_t reg_field = static_cast<uint8_t>(reg_num & 0x3F);
                if (!suffix.empty()) {
                    if (suffix == "H")
                        reg_field |= 0x80;
                    else if (suffix == "L")
                        reg_field |= 0x40;
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
                std::string inside = chosen_token->data;
                if (!inside.empty() && inside.front() == '[')
                    inside.erase(0, 1);
                if (!inside.empty() && inside.back() == ']')
                    inside.pop_back();
                trim(inside);
                if (!inside.empty() && inside[0] == '#')
                    inside.erase(0, 1);
                try {
                    value_to_store = std::stoll(inside, nullptr, 0);
                } catch (const std::invalid_argument &) {
                    std::cerr << "ERROR: Invalid memory address '" << inside << "'\n";
                    continue;
                }
                break;
            }
            case OperandSubtype::OffsetMemory: {
                auto [base_val, offset_val] = parse_offset_memory_subfields(chosen_token->data);
                if (sub_field == "baseReg") {
                    if (base_val < 0 || base_val > 63) {
                        std::cerr << "ERROR: Base register number '" << base_val
                                  << "' out of range (0-63).\n";
                        continue;
                    }
                    value_to_store = static_cast<uint8_t>(base_val & 0x3F);
                } else if (sub_field == "offset") {
                    value_to_store = static_cast<uint64_t>(offset_val);
                } else {
                    std::cerr << "ERROR: Unknown subfield '" << sub_field
                              << "' for OffsetMemory.\n";
                    continue;
                }
                break;
            }
            case OperandSubtype::LabelReference: {
                // Here we are processing a label reference.
                // We write a dummy 32-bit placeholder (0) into the object code.
                // Record a relocation entry so that the linker (or a second pass)
                // can patch this location with the actual address.
                // Assume we have access to a relocation_entries vector.
                // The current position is where these bytes will be inserted.
                value_to_store = this->label_table[chosen_token->data];
                auto patch_position = static_cast<uint32_t>(object_code.size());
                // Note: you may want to strip any extra punctuation from the token,
                // so that token.data contains just the label name.
                // Also note that the relocation entry’s address field is defined as the
                // location in the object code to be patched.
                // For example:
                this->relocation_entries.emplace_back(chosen_token->data, patch_position);
                break;
            }
            default:
                std::cerr << "ERROR: Unhandled operand subtype for token '"
                          << chosen_token->data << "'\n";
                continue;
        }

        store_big_endian(field_bytes, value_to_store);
        object_code.insert(object_code.end(), field_bytes.begin(), field_bytes.end());
    }
}

// Get operand fields and their bit widths from the machine description.
std::vector<std::pair<std::string, uint8_t> >
CodeGenerator::get_operand_lengths(const std::string &inst_name, uint8_t sp) {
    const char *encoding = get_encoding_for_instruction(inst_name.c_str(), sp);
    if (!encoding)
        return {};

    std::string enc_str(encoding);
    std::istringstream iss(enc_str);
    std::string token_str;
    std::vector<std::pair<std::string, uint8_t> > fields;

    while (iss >> token_str) {
        if (!token_str.empty() && token_str.front() == '[')
            token_str.erase(0, 1);
        if (!token_str.empty() && token_str.back() == ']')
            token_str.pop_back();

        auto paren_open = token_str.find('(');
        auto paren_close = token_str.find(')');
        if (paren_open == std::string::npos || paren_close == std::string::npos ||
            paren_close < paren_open)
            continue;

        std::string field_name = token_str.substr(0, paren_open);
        std::string width_str = token_str.substr(paren_open + 1, paren_close - paren_open - 1);
        auto bit_width = static_cast<uint8_t>(std::stoi(width_str));

        if (field_name == "sp" || field_name == "opcode")
            continue;

        fields.emplace_back(field_name, bit_width);
    }

    return fields;
}

// Build a map linking operand placeholders to their tokens.
std::unordered_map<std::string, Token>
CodeGenerator::build_placeholder_map(const std::string &syntax_str,
                                       const std::vector<Token> &operand_tokens) {
    std::unordered_map<std::string, Token> placeholder_map;
    auto first_space = syntax_str.find(' ');
    if (first_space == std::string::npos)
        return placeholder_map;

    std::string operand_part = syntax_str.substr(first_space + 1);
    std::vector<std::string> placeholders;
    std::istringstream iss(operand_part);
    std::string tmp_str;
    while (std::getline(iss, tmp_str, ',')) {
        tmp_str.erase(tmp_str.begin(), std::find_if(tmp_str.begin(), tmp_str.end(),
                                                     [](unsigned char ch) { return !std::isspace(ch); }));
        tmp_str.erase(std::find_if(tmp_str.rbegin(), tmp_str.rend(),
                                   [](unsigned char ch) { return !std::isspace(ch); }).base(), tmp_str.end());
        if (!tmp_str.empty())
            placeholders.push_back(tmp_str);
    }

    if (placeholders.size() != operand_tokens.size()) {
        std::cerr << "Mismatch between placeholder count and operand token count!\n";
        return placeholder_map;
    }

    for (size_t i = 0; i < placeholders.size(); i++) {
        placeholder_map[placeholders[i]] = operand_tokens[i];
    }

    return placeholder_map;
}

// Parse offset memory operands like "[2 + #8]".
std::pair<int, int> CodeGenerator::parse_offset_memory_subfields(const std::string &token_data) {
    auto cache_it = offset_memory_cache.find(token_data);
    if (cache_it != offset_memory_cache.end())
        return cache_it->second;

    std::string content = token_data.substr(1, token_data.size() - 2);
    auto plus_pos = content.find('+');
    if (plus_pos == std::string::npos)
        throw std::runtime_error("OffsetMemory operand missing '+': " + token_data);

    std::string base_str = content.substr(0, plus_pos);
    std::string offset_str = content.substr(plus_pos + 1);

    auto trim = [](std::string &s) {
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
            s.erase(s.begin());
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
            s.pop_back();
    };
    trim(base_str);
    trim(offset_str);

    int base_val = 0;
    if (!base_str.empty() && base_str[0] == '#')
        base_val = std::stoi(base_str.substr(1), nullptr, 0);
    else
        base_val = std::stoi(base_str, nullptr, 0);

    int offset_val = 0;
    if (!offset_str.empty() && offset_str[0] == '#')
        offset_val = std::stoi(offset_str.substr(1), nullptr, 0);
    else
        offset_val = std::stoi(offset_str, nullptr, 0);

    offset_memory_cache[token_data] = {base_val, offset_val};
    return {base_val, offset_val};
}

// Find the token for a field, handling register suffixes or offset memory.
const Token* CodeGenerator::find_token_for_field(const std::string &field_name,
                                                   const std::unordered_map<std::string, Token> &placeholder_map,
                                                   std::string &subFieldOut,
                                                   std::string &regSuffixOut) {
    subFieldOut.clear();
    regSuffixOut.clear();

    bool is_register_field =
        (field_name == "rd" || field_name == "rd1" ||
         field_name == "rn" || field_name == "rn1" ||
         field_name == "rm" || field_name == "rs");

    if (is_register_field) {
        std::string directKey = "%" + field_name;
        auto it = placeholder_map.find(directKey);
        if (it != placeholder_map.end())
            return &it->second;

        for (const auto &kv : placeholder_map) {
            const std::string &ph = kv.first;
            if (ph.find(directKey + ".") == 0) {
                auto [mainPart, suffix] = split_register_suffix(ph.substr(1));
                if (mainPart == field_name) {
                    regSuffixOut = suffix;
                    return &kv.second;
                }
            }
        }

        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::OffsetMemory) {
                subFieldOut = "baseReg";
                return &kv.second;
            }
        }

        return nullptr;
    }
    else if (field_name == "offset") {
        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::OffsetMemory) {
                subFieldOut = "offset";
                return &kv.second;
            }
        }
    }
    else if (field_name == "immediate" || field_name == "operand2") {
        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::Immediate  || kv.second.subtype == OperandSubtype::LabelReference)
                return &kv.second;
        }
    }
    else if (field_name == "normAddressing") {
        for (const auto &kv : placeholder_map) {
            if ((kv.second.subtype == OperandSubtype::Memory) || kv.second.subtype == OperandSubtype::LabelReference)
                return &kv.second;
        }
    }
    else if (field_name == "label") {
        for (const auto &kv : placeholder_map) {
            if (kv.second.subtype == OperandSubtype::LabelReference)
                return &kv.second;
        }
    }

    return nullptr;
}