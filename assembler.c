//
// Created by dulat on 1/1/23.
//
#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

uint8_t get_opcode(const char* instruction) {
    printf("%s\n", instruction);
    if (strcmp(instruction, "NOP") == 0) {
        return 0x00;
    } else if (strcmp(instruction, "ADD") == 0) {
        return 0x01;
    } else if (strcmp(instruction, "SUB") == 0) {
        return 0x02;
    } else if (strcmp(instruction, "MUL") == 0) {
        return 0x03;
    } else if (strcmp(instruction, "ADM") == 0) {
        return 0x04;
    } else if (strcmp(instruction, "SBM") == 0) {
        return 0x05;
    } else if (strcmp(instruction, "MLM") == 0) {
        return 0x06;
    } else if (strcmp(instruction, "ADR") == 0) {
        return 0x07;
    } else if (strcmp(instruction, "SBR") == 0) {
        return 0x08;
    } else if (strcmp(instruction, "MLR") == 0) {
        return 0x09;
    } else if (strcmp(instruction, "CLZ") == 0) {
        return 0x0A;
    } else if (strcmp(instruction, "STO") == 0) {
        return 0x0B;
    } else if (strcmp(instruction, "STM") == 0) {
        return 0x0C;
    } else if (strcmp(instruction, "LDM") == 0) {
        return 0x0D;
    } else if (strcmp(instruction, "PSH") == 0) {
        return 0x0E;
    } else if (strcmp(instruction, "POP") == 0) {
        return 0x0F;
    } else if (strcmp(instruction, "PRT") == 0) {
        return 0x10;
    } else if (strcmp(instruction, "RDM") == 0) {
        return 0x11;
    } else if (strcmp(instruction, "RNM") == 0) {
        return 0x12;
    } else if (strcmp(instruction, "BRN") == 0) {
        return 0x13;
    } else if (strcmp(instruction, "BRZ") == 0) {
        return 0x14;
    } else if (strcmp(instruction, "BRO") == 0) {
        return 0x15;
    } else if (strcmp(instruction, "BRR") == 0) {
        return 0x16;
    } else if (strcmp(instruction, "BNR") == 0) {
        return 0x17;
    } else if (strcmp(instruction, "HLT") == 0) {
        return 0x18;
    } else {
        printf("Invalid instruction\n");
        return SENTINEL_VALUE;
    }
}

uint8_t get_operand(const char* operand) {
    // Check if the operand is a register
    if (strcmp(operand, "0") == 0) {
        return 0;
    } else if (strcmp(operand, "1") == 0) {
        return 1;
    }
    printf("Operand2: %s\n", operand);
    // If the operand is not a register, it must be an immediate value
    return (uint16_t) strtol(operand, NULL, 16);
}

// Function to lex the input string into a list of tokens
Token* lex(const char* input) {
    // Allocate memory for the token array
    Token* tokens = calloc(1, sizeof(Token));
    int token_count = 0;

    // Read the input character by character
    volatile int i = 0;
    while (input[i] != '\0' && i < 1024) {
        // Skip over whitespace
        if (input[i] == ' ' || input[i] == '\t' || input[i] == '\n' || input[i] == ',') {
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
            while (isxdigit(input[i]) && j < MAX_TOKEN_LEN) {
                tokens[token_count].value[j] = input[i];
                i++;
                j++;
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = 3; // Set the token type to 3 for immediate values
            token_count++;
            continue;
        }
        // Check for a register
        if (input[i] == '0' || input[i] == '1') {
            // Allocate memory for the new token
            tokens = realloc(tokens, sizeof(Token) * (token_count + 1));

            // Read the immediate value into the new token
            /*
            int j = 0;
            while ( && j < MAX_TOKEN_LEN) {
                tokens[token_count].value[j] = input[i];
                i++;
                j++;
            }
            tokens[token_count].value[j] = '\0';*/
            tokens[token_count].value[0] = input[i];
            tokens[token_count].value[1] = '\0';
            tokens[token_count].type = 2; // Set the token type to 2 for register values
            token_count++;
            i++;
            continue;
        }
        // Check for labels
        if (input[i] == '.') {
            tokens = realloc(tokens, sizeof(Token) * (token_count + 1));
            int j = 0;
            while (input[i] != '\n' && input[i] != '\0' && i < 1024) {
                tokens[token_count].value[j] = input[i];
                i++;
                j++;
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = 4; // Set the token type to 4 for labels
            token_count++;
            continue;
        }

        // If none of the above cases are true, the input is invalid
        printf("Invalid input at line: \n");
        printf("%c", input[i]);
        return NULL;
    }

    // Add a sentinel token to the end of the list
    tokens = realloc(tokens, sizeof(Token) * (token_count + 1));
    tokens[token_count].value[0] = '\0';
    tokens[token_count].type = -1;

    return tokens;
}

// Function to parse the tokens into a list of instructions
void parse(Instruction *instructions, Token *tokens, uint8_t current_token, Labels *label_addresses,
           size_t *current_size) {
    // Allocate memory for the instruction array
    //instructions = realloc(instructions, sizeof(Instruction*)*2);
    if (tokens[0].type == 1) {

        // Get the opcode for the instruction or label
        instructions->opcode = get_opcode(tokens[0].value);
        // Initialize the operands to default values
        //instructions->operand1 = get_operand(tokens[1].value);
        //instructions->operand2 = get_operand(tokens[2].value);
        // Check for operands
        if (tokens[1].type == 2 && tokens[2].type == 2 && tokens[3].type == 3) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = get_operand(tokens[2].value);
            instructions->operand2 = get_operand(tokens[3].value);
        } else if (tokens[1].type == 2 && tokens[2].type == 3) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = 0;
            instructions->operand2 = get_operand(tokens[2].value);
        } else if (tokens[1].type == 2 && tokens[2].type == 2) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = get_operand(tokens[2].value);
            instructions->operand2 = 0;
        } else if (tokens[1].type == 2) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = 0;
            instructions->operand2 = 0;
        } else if (tokens[1].type == 4 && ((instructions->opcode == 0x13) || (instructions->opcode == 0x14) || (instructions->opcode == 0x15))) {
            instructions->operand_rd = 0;
            instructions->operand_rn = 0;
            for(int i = 0; i < (int)sizeof(*label_addresses)/(int)sizeof(label_addresses[0]); i++) {
                if(strcmp(label_addresses[i].label, tokens[3].value) == 0){
                    instructions->operand2 = label_addresses[i].address;
                }
                else if(i==(int)sizeof(*label_addresses)/(int)sizeof(label_addresses[0])-1){
                    fprintf(stderr, "Error: Label %s not found\n", tokens[3].value);
                }
            }
        } else if (tokens[1].type == 2 && tokens[2].type == 2 && tokens[3].type == 4 && (instructions->opcode == 0x16 || instructions->opcode == 0x17)) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = get_operand(tokens[2].value);
            for(int i = 0; i < (int)sizeof(*label_addresses)/(int)sizeof(label_addresses[0]); i++) {
                if(strcmp(label_addresses[i].label, tokens[3].value) == 0){
                    instructions->operand2 = label_addresses[i].address;
                }
                else if(i==(int)sizeof(*label_addresses)/(int)sizeof(label_addresses[0])-1){
                    fprintf(stderr, "Error: Label %s not found\n", tokens[3].value);
                }
            }
        } else {
            instructions->operand_rd = 0;
            instructions->operand_rn = 0;
            instructions->operand2 = 0;
        }
        return;
    } else if (tokens[0].type == 4) {
        instructions->opcode = get_opcode("NOP");
        label_addresses = realloc(label_addresses, (*current_size+1) * sizeof(Labels));
        struct Labels labels;
        strcpy(labels.label, tokens[0].value);
        labels.address = current_token;
        label_addresses[*current_size] = labels;
        (*current_size)++;
        instructions->operand_rd = 0;
        instructions->operand_rn = 0;
        instructions->operand2 = 0;
    } else {
        printf("Invalid token\n");
        return;
    }
}

// Function to generate machine code from the instructions
uint16_t* generate_code(Instruction* instructions, uint16_t instruction_count) {
    // Allocate memory for the machine code array
    uint16_t* code = malloc(sizeof(uint16_t) * 255);
    memset(code, 0, 255);
    int16_t code_len = 0;

    // Generate the machine code for each instruction
    while (code_len < instruction_count) {
        // Pack the opcode, operand1, and operand2 fields into a single 16-bit word
        uint16_t instruction_word = instructions[code_len].opcode;
        instruction_word |= (instructions[code_len].operand_rd << 6);
        instruction_word |= (instructions[code_len].operand_rn << 7);
        instruction_word |= (instructions[code_len].operand2 << 8);

        // Write the instruction word to the machine code array
        code[code_len] = instruction_word;
        code_len++;
    }

    // Return the machine code array
    return code;
}