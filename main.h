//
// Created by dulat on 1/1/23.
//

#ifndef CPU_ASSEMBLER_MAIN_H
#define CPU_ASSEMBLER_MAIN_H

#endif //CPU_ASSEMBLER_MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <arpa/inet.h>

#define MAX_TOKEN_LEN 16
#define MAX_INSTRUCTION_LEN 3
#define SENTINEL_VALUE 255

#define TYPE_OPCODE 1
#define TYPE_REGISTER 2
#define TYPE_OPERAND_2 3
#define TYPE_LABEL 4
#define PROGRAM_WORDS 256

#define OP_NOP 0x00
#define OP_ADD 0x01
#define OP_SUB 0x02
#define OP_MUL 0x03
#define OP_ADM 0x04
#define OP_SBM 0x05
#define OP_MLM 0x06
#define OP_ADR 0x07
#define OP_SBR 0x08
#define OP_MLR 0x09
#define OP_CLZ 0x0A
#define OP_STO 0x0B
#define OP_STM 0x0C
#define OP_LDM 0x0D
#define OP_PSH 0x0E
#define OP_POP 0x0F
#define OP_BRN 0x10
#define OP_BRZ 0x11
#define OP_BRO 0x12
#define OP_BRR 0x13
#define OP_BNR 0x14
#define OP_HLT 0x15
#define OP_TSK 0x16
#define OP_SCH 0x17
#define OP_SWT 0x18
#define OP_KIL 0x19

#define MAX_CODE_LENGTH 256

// Structure to represent a token
typedef struct Token {
    char value[MAX_TOKEN_LEN + 1];
    int type;
} Token;

// Structure to represent an instruction
typedef struct Instruction {
    uint8_t opcode;
    uint8_t operand_rd;
    uint8_t operand_rn;
    uint16_t operand2;
} Instruction;

typedef struct Labels {
    char label[MAX_TOKEN_LEN + 1];
    uint8_t address;
} Labels;

// Function prototypes
Token* lex(const char* input, uint8_t current_token);
void parse(Instruction *instructions, const Token *tokens, const uint8_t *current_token, Labels **label_addresses,
           size_t *current_size, const uint8_t *conf, size_t confSize);

uint8_t get_opcode(const char* instruction, const uint8_t* conf, size_t confSize);
uint16_t get_operand(const char* operand);
uint8_t *generate_code(const Instruction *instructions, uint8_t instruction_count, const uint8_t *conf, size_t confSize);
void write_code(uint8_t *code, uint16_t code_len, const char* filename);
void *realloc_zero(void *ptr, size_t old_size, size_t new_size);
void get_label(Labels **label_addresses, const char label[MAX_TOKEN_LEN + 1], uint8_t current_token, uint8_t *current_size);

uint8_t num_operands(uint8_t opcode, const uint8_t *conf, size_t confSize);
uint8_t operand1_mode(uint8_t opcode, const uint8_t *conf, size_t confSize);