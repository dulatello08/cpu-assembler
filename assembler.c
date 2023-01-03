//
// Created by ducat on 1/1/23.
//
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

uint8_t get_opcode(const char* instruction) {
    printf("%s", instruction);
    if (strcmp(instruction, "NOP") == 0) {
        return 0x00;
    } else if (strcmp(instruction, "ADD") == 0) {
        return 0x01;
    } else if (strcmp(instruction, "SUB") == 0) {
        return 0x02;
    } else if (strcmp(instruction, "MUL") == 0) {
        return 0x03;
    } else if (strcmp(instruction, "CLZ") == 0) {
        return 0x04;
    } else if (strcmp(instruction, "ADM") == 0) {
        return 0x05;
    } else if (strcmp(instruction, "SBM") == 0) {
        return 0x06;
    } else if (strcmp(instruction, "STO") == 0) {
        return 0x07;
    } else if (strcmp(instruction, "STM") == 0) {
        return 0x08;
    } else if (strcmp(instruction, "LDM") == 0) {
        return 0x09;
    } else if (strcmp(instruction, "PSH") == 0) {
        return 0x0A;
    } else if (strcmp(instruction, "POP") == 0) {
        return 0x0B;
    } else if (strcmp(instruction, "BRN") == 0) {
        return 0x0C;
    } else if (strcmp(instruction, "BRZ") == 0) {
        return 0x0D;
    } else if (strcmp(instruction, "BRO") == 0) {
        return 0x0E;
    } else if (strcmp(instruction, "HLT") == 0) {
        return 0x0F;
    } else {
        printf("Invalid instruction\n");
        return SENTINEL_VALUE;
    }
}

uint8_t get_operand(const char* operand) {
    // Check if the operand is a register
    if (operand[0] == 'A') {
        return 0;
    } else if (operand[0] == 'B') {
        return 1;
    }

    // If the operand is not a register, it must be an immediate value
    return (uint8_t) strtol(operand, NULL, 10);
}

// Function to lex the input string into a list of tokens
Token* lex(const char* input) {
    // Allocate memory for the token array
    Token* tokens = malloc(sizeof(Token));
    int token_count = 0;

    // Read the input character by character
    volatile int i = 0;
    while (input[i] != '\0' && i < 1024) {
        // Skip over whitespace
        if (input[i] == ' ' || input[i] == '\t' || input[i] == '\n') {
            i++;
            continue;
        }

        // Check for a comment
        if (input[i] == ';') {
            // Skip to the end of the line
            while (input[i] != '\n' && input[i] != '\0' && i < 1024) {
                i++;
            }
            continue;
        }
        // Check for an instruction or directive
        if (isalpha(input[i])) {
            // Allocate memory for the new token
            tokens = realloc(tokens, sizeof(Token) * (token_count + 1));

            // Read the instruction or directive into the new token
            int j = 0;
            while (isalpha(input[i]) && j < MAX_INSTRUCTION_LEN) {
                printf("%d\n", i);
                tokens[token_count].value[j] = input[i];
                i++;
                j++;
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = 1; // Set the token type to 1 for instructions and directives
            token_count++;
            continue;
        }

        // Check for an immediate value
        if (input[i] == '#') {
            // Allocate memory for the new token
            tokens = realloc(tokens, sizeof(Token) * (token_count + 1));

            // Read the immediate value into the new token
            i++;
            int j = 0;
            while (isdigit(input[i]) && j < MAX_TOKEN_LEN) {
                tokens[token_count].value[j] = input[i];
                i++;
                j++;
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = 2; // Set the token type to 2 for immediate values
            token_count++;
            continue;
        }
        // Check for a register
        if (input[i] == '&') {
            // Allocate memory for the new token
            tokens = realloc(tokens, sizeof(Token) * (token_count + 1));

            // Read the immediate value into the new token
            i++;
            int j = 0;
            while (isalpha(input[i]) && j < MAX_TOKEN_LEN) {
                tokens[token_count].value[j] = input[i];
                i++;
                j++;
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = 3; // Set the token type to 2 for register values
            token_count++;
            continue;
        }

        // If none of the above cases are true, the input is invalid
        printf("Invalid input\n");
        return NULL;
    }

    // Add a sentinel token to the end of the list
    tokens = realloc(tokens, sizeof(Token) * (token_count + 1));
    tokens[token_count].value[0] = '\0';
    tokens[token_count].type = -1;

    return tokens;
}

// Function to parse the tokens into a list of instructions
Instruction* parse(const Token* tokens) {
    // Allocate memory for the instruction array
    Instruction* instructions = malloc(sizeof(Instruction));
    int instruction_count = 0;

    // Read the tokens one by one
    int i = 0;
    while (tokens[i].type != -1) {
        // Check for an instruction or directive
        if (tokens[i].type == 1) {
            // Allocate memory for the new instruction
            instructions = realloc(instructions, sizeof(Instruction) * (instruction_count + 1));

            // Get the opcode for the instruction
            instructions[instruction_count].opcode = get_opcode(tokens[i].value);

            // Initialize the operands to default values
            instructions[instruction_count].operand1 = get_operand(tokens[i+1].value);
            instructions[instruction_count].operand2 = get_operand(tokens[i+2].value);
            /*// Check for an operand
            if (tokens[i + 1].type == 2 || tokens[i + 1].type == 3) {
                instructions[instruction_count].operand1 = 1;
                instructions[instruction_count].operand2 = get_operand(tokens[i + 1].value);
                i += 2;
            } else {
                i++;
            }*/
            i += 3;
            instruction_count++;
        } else {
            printf("Invalid token\n");
            return NULL;
        }
    }

    // Add a sentinel instruction to the end of the list
    instructions = realloc(instructions, sizeof(Instruction) * (instruction_count + 1));
    instructions[instruction_count].opcode = -1;

    return instructions;
}

// Function to generate machine code from the instructions
uint16_t* generate_code(const Instruction* instructions) {
    // Allocate memory for the machine code array
    uint16_t* code = malloc(sizeof(uint16_t) * 255);
    int16_t code_len = 0;

    // Generate the machine code for each instruction
    int i = 0;
    while (instructions[i].opcode != SENTINEL_VALUE) {
        // Pack the opcode, operand1, and operand2 fields into a single 16-bit word
        uint16_t instruction_word = instructions[i].opcode;
        instruction_word |= (instructions[i].operand1 << 7);
        instruction_word |= (instructions[i].operand2 << 8);

        // Write the instruction word to the machine code array
        code[code_len] = instruction_word;
        code_len++;

        i++;
    }

    // Return the machine code array
    return code;
}