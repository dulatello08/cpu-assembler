//
// Created by Dulat S on 1/20/25.
//
#include <cstring>    // For strcmp
#include "machine_description.h"

// Retrieve opcode for a given instruction name.
uint8_t get_opcode_for_instruction(const char* inst_name) {
    size_t num_instructions = sizeof(instructions) / sizeof(instructions[0]);
    for (size_t i = 0; i < num_instructions; ++i) {
        if (strcmp(instructions[i].name, inst_name) == 0) {
            return instructions[i].opcode;
        }
    }
    // Not found: return a special value or handle error
    return 0xFF;
}

// Helper to find the InstructionFormat for a given instruction name.
const InstructionFormat* find_instruction_format(const char* inst_name) {
    size_t num_instructions = sizeof(instructions) / sizeof(instructions[0]);
    for (size_t i = 0; i < num_instructions; ++i) {
        if (strcmp(instructions[i].name, inst_name) == 0) {
            return &instructions[i];
        }
    }
    return nullptr;
}

// Retrieve syntax based on instruction name and specifier 'sp'.
const char* get_syntax_for_instruction(const char* inst_name, uint8_t sp) {
    const InstructionFormat* format = find_instruction_format(inst_name);
    if (!format) return nullptr;
    for (size_t i = 0; i < format->num_specifiers; ++i) {
        if (format->specifiers[i].sp == sp) {
            return format->specifiers[i].syntax;
        }
    }
    return nullptr;
}

// Retrieve encoding based on instruction name and specifier 'sp'.
const char* get_encoding_for_instruction(const char* inst_name, uint8_t sp) {
    const InstructionFormat* format = find_instruction_format(inst_name);
    if (!format) return nullptr;
    for (size_t i = 0; i < format->num_specifiers; ++i) {
        if (format->specifiers[i].sp == sp) {
            return format->specifiers[i].encoding;
        }
    }
    return nullptr;
}

// Retrieve length based on instruction name and specifier 'sp'.
uint8_t get_length_for_instruction(const char* inst_name, uint8_t sp) {
    const InstructionFormat* format = find_instruction_format(inst_name);
    if (!format) return 0;
    for (size_t i = 0; i < format->num_specifiers; ++i) {
        if (format->specifiers[i].sp == sp) {
            return format->specifiers[i].length;
        }
    }
    return 0;
}