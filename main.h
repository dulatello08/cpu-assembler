//
// Created by dulat on 1/1/23.
//

#ifndef CPU_ASSEMBLER_MAIN_H
#define CPU_ASSEMBLER_MAIN_H

#endif //CPU_ASSEMBLER_MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>

#define MAX_TOKEN_LEN 16
#define MAX_INSTRUCTION_LEN 3
#define SENTINEL_VALUE 255

// Structure to represent a token
typedef struct Token {
    char value[MAX_TOKEN_LEN];
    int type;
} Token;

// Structure to represent an instruction
typedef struct Instruction {
    uint8_t opcode;
    bool operand_rd;
    bool operand_rn;
    uint8_t operand2;
} Instruction;

typedef struct Labels {
    char label[MAX_TOKEN_LEN];
    uint8_t address;
} Labels;

// Function prototypes
Token* lex(const char* input, uint8_t current_token);
void parse(Instruction *instructions, Token *tokens, uint8_t current_token, Labels *label_addresses,
           size_t *current_size);

uint8_t get_opcode(const char* instruction);
uint8_t get_operand(const char* operand);
uint16_t* generate_code(Instruction* instructions, uint16_t instruction_count);
void write_code(uint16_t* code, int16_t code_len, const char* filename);
uint16_t* get_code_len(Instruction** instructions);