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
        if (const Token &current_token = tokens[currentTokenIndex]; current_token.type == TokenType::Label) {
            currentTokenIndex++;
        } else if (current_token.type == TokenType::Instruction) {
            try {
                // Parse the instruction and get the mnemonic and specifier syntax.
                parse_instruction();
            } catch (const std::exception &e) {
                std::cerr << e.what() << "\n";
            }
        } else {
            std::cerr << "Unexpected token: " << current_token.data
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
            break;
        }
        operand_tokens.push_back(lookahead);
        currentTokenIndex++;
    }

    // 3) Find the instruction format
    const InstructionFormat *instruction_format = find_instruction_format(inst_name.c_str());
    if (!instruction_format) {
        throw std::runtime_error("Unknown instruction: " + inst_name);
    }

    // Try each specifier until we find one that matches the operand pattern:
    const InstructionSpecifier *chosen_spec = nullptr;
    for (size_t i = 0; i < instruction_format->num_specifiers; ++i) {
        const InstructionSpecifier &spec = instruction_format->specifiers[i];
        if (match_operands_against_syntax(operand_tokens, spec.syntax)) {
            chosen_spec = &spec;
            break;
        }
    }
    if (!chosen_spec) {
        throw std::runtime_error(
            "No matching syntax for '" + inst_name + "' with given operands."
        );
    }

    // Finally, assemble:
    assemble_instruction(chosen_spec, inst_name, operand_tokens);
}

void Parser::assemble_instruction(const InstructionSpecifier *spec,
                                   const std::string &inst_name,
                                   const std::vector<Token> &operand_tokens)
{
    // 1) Write out 'sp' and 'opcode'
    object_code.push_back(static_cast<uint8_t>(spec->sp));
    uint8_t opcode = get_opcode_for_instruction(inst_name.c_str());
    object_code.push_back(opcode);

    // 2) Build a map from placeholder => token, as always.
    auto placeholder_map = build_placeholder_map(spec->syntax, operand_tokens);

    // 3) Get the operand fields from the machine-encoding definition
    auto operand_fields = get_operand_lengths(inst_name, spec->sp);
    // convert bits -> bytes
    for (auto &operand_field_pair : operand_fields) {
        operand_field_pair.second = static_cast<uint8_t>((operand_field_pair.second + 7) / 8);
    }

    // 4) For each field in the encoding, figure out which token to use
    for (auto &[field_name, field_byte_width] : operand_fields)
    {
        // We create a buffer of 'field_byte_width' zeros:
        std::vector<uint8_t> field_bytes(field_byte_width, 0);

        // 4A) Decide which placeholder + subcomponent we need for this field_name.
        //     "rd", "rn", "immediate", "baseReg", "offset", "normAddressing", etc.
        //
        //     For instance, if it's "baseReg" or "offset", we search for the *one* offsetMemory token in placeholder_map.
        //     If it's "rd", we search for the *one* register placeholder, etc.
        //
        // NOTE: This is a minimal example. For a real assembler, you'd do a more robust approach
        // that accounts for multiple offsetMemory operands, or multiple registers, etc.

        const Token* chosen_token = nullptr;
        std::string sub_field; // e.g. "baseReg" or "offset"

        // Example 1: "rd" => find a placeholder which is a register
        if (field_name == "rd" || field_name == "rn") {
            // Look for a placeholder in placeholder_map whose token.subtype == Register
            for (auto &key_value : placeholder_map) {
                if (key_value.second.subtype == OperandSubtype::Register) {
                    chosen_token = &key_value.second;
                    break;
                }
            }
            if (!chosen_token) {
                std::cerr << "No Register operand found for field '" << field_name << "'\n";
                continue;
            }
            // We'll encode that register as normal below.

        }
        // Example 2: "baseReg" => from an OffsetMemory operand
        else if (field_name == "baseReg") {
            // Find offsetMemory operand in placeholder_map:
            for (auto &key_value : placeholder_map) {
                if (key_value.second.subtype == OperandSubtype::OffsetMemory) {
                    chosen_token = &key_value.second;
                    sub_field    = "baseReg";
                    break;
                }
            }
            if (!chosen_token) {
                std::cerr << "No offsetMemory operand found for 'baseReg'\n";
                continue;
            }
        }
        // Example 3: "offset" => from that same OffsetMemory token
        else if (field_name == "offset") {
            for (auto &key_value : placeholder_map) {
                if (key_value.second.subtype == OperandSubtype::OffsetMemory) {
                    chosen_token = &key_value.second;
                    sub_field    = "offset";
                    break;
                }
            }
            if (!chosen_token) {
                std::cerr << "No offsetMemory operand found for 'offset'\n";
                continue;
            }
        }
        // Example 4: "immediate"
        else if (field_name == "immediate" || field_name == "imm" || field_name == "operand2")
        {
            // Or whatever your real field names are for immediate...
            // Find a placeholder that is Immediate
            for (auto &key_value : placeholder_map) {
                if (key_value.second.subtype == OperandSubtype::Immediate) {
                    chosen_token = &key_value.second;
                    break;
                }
            }
            if (!chosen_token) {
                std::cerr << "No immediate operand found for '" << field_name << "'\n";
                continue;
            }
        }
        // Example 5: "normAddressing" => from a Memory operand
        else if (field_name == "normAddressing") {
            // find token whose subtype == Memory
            for (auto &key_value : placeholder_map) {
                if (key_value.second.subtype == OperandSubtype::Memory) {
                    chosen_token = &key_value.second;
                    break;
                }
            }
            if (!chosen_token) {
                std::cerr << "No Memory operand found for 'normAddressing'\n";
                continue;
            }
        }
        else {
            // If your architecture has other field names, handle them similarly.
            // Or skip. We'll just log a warning:
            std::cerr << "No placeholder logic for field '" << field_name
                      << "' => skipping.\n";
            continue;
        }

        // 4B) Now actually encode that chosen_token into field_bytes.
        //     If sub_field == "baseReg"/"offset", we parse partial info from the token.
        //     Otherwise, we encode the entire token as usual.
        if (!chosen_token) {
            continue; // we had an error above
        }

        if (chosen_token->subtype == OperandSubtype::Immediate) {
            // e.g. token data is "#123"
            int value = std::stoi(chosen_token->data.substr(1));
            for (int b = 0; b < static_cast<int>(field_byte_width); ++b) {
                field_bytes[b] = (value >> (8 * (field_byte_width - 1 - b))) & 0xFF;
            }
        }
        else if (chosen_token->subtype == OperandSubtype::Register) {
            // e.g. token data is "%3", or "3", or "3.H"
            std::string reg = chosen_token->data;
            // remove prefix/suffix:
            if (!reg.empty() && reg[0] == '%') {
                reg.erase(0,1);
            }
            if (auto pos = reg.find(".H"); pos != std::string::npos) {
                reg = reg.substr(0, pos);
            }
            else if (auto pos = reg.find(".L"); pos != std::string::npos) {
                reg = reg.substr(0, pos);
            }
            int reg_num = std::stoi(reg);
            // put reg_num in the first byte (or multiple bytes as needed):
            for (int b = 0; b < static_cast<int>(field_byte_width); ++b) {
                // Usually this is just 1 byte if reg is small
                field_bytes[field_byte_width - 1 - b] = (reg_num >> (8 * b)) & 0xFF;
            }
        }
        else if (chosen_token->subtype == OperandSubtype::Memory) {
            // e.g. "[#0x100]"
            // remove brackets:
            std::string inside = chosen_token->data.substr(1, chosen_token->data.size() - 2);
            // maybe inside is "#0x100"
            if (!inside.empty() && inside[0] == '#') {
                inside.erase(0,1); // remove '#'
            }
            int mem_val = std::stoi(inside, nullptr, 0);
            for (int b = 0; b < static_cast<int>(field_byte_width); ++b) {
                field_bytes[b] = (mem_val >> (8 * (field_byte_width - 1 - b))) & 0xFF;
            }
        }
        else if (chosen_token->subtype == OperandSubtype::OffsetMemory) {
            // e.g. "[2 + #8]"
            auto [base_val, offset_val] = parse_offset_memory_subfields(chosen_token->data);

            int value_to_store = 0;
            if (sub_field == "baseReg") {
                value_to_store = base_val;
            }
            else if (sub_field == "offset") {
                value_to_store = offset_val;
            }
            // place value_to_store into field_bytes:
            for (int b = 0; b < static_cast<int>(field_byte_width); ++b) {
                field_bytes[b] = (value_to_store >> (8 * (field_byte_width - 1 - b))) & 0xFF;
            }
        }
        else {
            std::cerr << "Unknown or unhandled operand subtype for token '"
                      << chosen_token->data << "'\n";
        }

        // Finally, append field_bytes to object_code:
        object_code.insert(object_code.end(), field_bytes.begin(), field_bytes.end());
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

    auto first_space = syntax_str.find(' ');
    if (first_space == std::string::npos) {
        // means there's no space => probably no operands
        // so if we have no operand tokens, match is true; else false
        return operand_tokens.empty();
    }

    // substring after the mnemonic
    // e.g. from "mov %rd, #%immediate" => "%rd, #%immediate"
    std::string operand_part = syntax_str.substr(first_space + 1);

    // 2) Split that on commas:
    std::vector<std::string> syntax_operands; {
        std::istringstream iss(operand_part);
        std::string temp;
        while (std::getline(iss, temp, ',')) {
            // trim leading/trailing space
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.front()))) temp.erase(temp.begin());
            while (!temp.empty() && isspace(static_cast<unsigned char>(temp.back()))) temp.pop_back();
            syntax_operands.push_back(temp);
        }
    }

    // If the number of syntax operands doesn't match the number of actual operand tokens, no match
    if (syntax_operands.size() != operand_tokens.size()) {
        return false;
    }

    // 3) Compare each syntax operand placeholder against the token’s subtype
    for (size_t i = 0; i < syntax_operands.size(); i++) {
        const std::string &placeholder = syntax_operands[i];
        const Token &actual_token = operand_tokens[i];

        // e.g. placeholder = "%rd", "#%immediate", "[%mem]", ...
        // check if it’s a register placeholder, immediate placeholder, memory, etc.
        if (!placeholder_matches_token(placeholder, actual_token)) {
            return false; // mismatch
        }
    }

    // If all placeholders matched, return true
    return true;
}

bool Parser::placeholder_matches_token(const std::string &placeholder, const Token &token) {
    // Handle Offset Memory Addressing
    if (placeholder.find("[%rn + #%offset]") != std::string::npos) {
        return (token.subtype == OperandSubtype::OffsetMemory);
    }
    // Handle Register Placeholders
    else if (placeholder.find("%rd") != std::string::npos ||
             placeholder.find("%rn") != std::string::npos ||
             placeholder.find("%rd1") != std::string::npos ||
             placeholder.find("%rn1") != std::string::npos) {
        // Ensure the token is a Register
        if (token.subtype != OperandSubtype::Register) {
            return false;
        }

        // Check for ".H" or ".L" in the placeholder
        bool expects_high = placeholder.find(".H") != std::string::npos;
        bool expects_low = placeholder.find(".L") != std::string::npos;

        // Check for ".H" or ".L" in the token's data
        bool has_high = token.data.find(".H") != std::string::npos;
        bool has_low = token.data.find(".L") != std::string::npos;

        if (expects_high) {
            return has_high;
        } else if (expects_low) {
            return has_low;
        } else {
            // Placeholder does not specify ".H" or ".L"
            // Ensure token's data does not contain either
            return !has_high && !has_low;
        }
    }
    // Handle Immediate Operands
    else if (placeholder.find("#%immediate") != std::string::npos) {
        return (token.subtype == OperandSubtype::Immediate);
    }
    // Handle Normal Memory Addressing
    else if (placeholder.find("[%normAddressing]") != std::string::npos) {
        return (token.subtype == OperandSubtype::Memory);
    }
    // If no known placeholder matched, return false
    return false;
}

std::vector<std::pair<std::string, uint8_t> >
Parser::get_operand_lengths(const std::string &inst_name, uint8_t sp) {
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

std::unordered_map<std::string, Token>
Parser::build_placeholder_map(const std::string &syntax_str,
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
    std::vector<std::string> placeholders; {
        std::istringstream iss(operand_part);
        std::string tmp_str;
        while (std::getline(iss, tmp_str, ',')) {
            // trim whitespace
            while (!tmp_str.empty() && std::isspace((unsigned char) tmp_str.front())) {
                tmp_str.erase(tmp_str.begin());
            }
            while (!tmp_str.empty() && std::isspace((unsigned char) tmp_str.back())) {
                tmp_str.pop_back();
            }
            if (!tmp_str.empty()) {
                placeholders.push_back(tmp_str);
            }
        }
    }

    // 3) Basic sanity check: number of placeholders == number of operand_tokens
    if (placeholders.size() != operand_tokens.size()) {
        // In a fully robust assembler, you'd handle this mismatch gracefully
        // or throw an exception. We'll just print an error for demonstration.
        std::cerr << "Mismatch between placeholder count and operand token count!\n";
        return placeholder_map;
    }

    // 4) Assign each placeholder to the corresponding Token in order.
    //    Because we've already validated the tokens with match_operands_against_syntax(...),
    //    we know operand_tokens[i] definitely matches placeholders[i].
    for (size_t i = 0; i < placeholders.size(); i++) {
        // Optionally, you could do an additional safety check:
        //    if (!placeholder_matches_token(placeholders[i], operand_tokens[i])) {
        //        // throw or log error
        //    }

        placeholder_map[placeholders[i]] = operand_tokens[i];
    }

    return placeholder_map;
}

// Helper: parse something like "[2 + #8]" => (base_reg=2, offset=8)
std::pair<int,int> Parser::parse_offset_memory_subfields(const std::string &token_data)
{
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
    std::string base_str   = content.substr(0, plus_pos);
    std::string offset_str = content.substr(plus_pos + 1);

    // Trim whitespace:
    auto trim = [](std::string &s){
        while(!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
        while(!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))  s.pop_back();
    };
    trim(base_str);
    trim(offset_str);

    // base_str might be "2" or "3", or possibly "#0x10" if your assembler design allows that.
    // offset_str might be "#8", "#0x10", etc.
    int base_val = 0;
    if (!base_str.empty() && base_str[0] == '#') {
        base_val = std::stoi(base_str.substr(1), nullptr, 0);
    } else {
        base_val = std::stoi(base_str, nullptr, 0);
    }

    int offset_val = 0;
    if (!offset_str.empty() && offset_str[0] == '#') {
        offset_val = std::stoi(offset_str.substr(1), nullptr, 0);
    } else {
        offset_val = std::stoi(offset_str, nullptr, 0);
    }

    offset_memory_cache[token_data] = {base_val, offset_val};
    return {base_val, offset_val};
}