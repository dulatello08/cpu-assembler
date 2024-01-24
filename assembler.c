//
// Created by dulat on 1/1/23.
//
#include "main.h"
#include <stdint.h>
#include <sys/types.h>

void *realloc_zero(void *ptr, size_t old_size, size_t new_size) {
    void *new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL) {
        // Handle allocation error
        fprintf(stderr, "Error: Failed to reallocate memory\n");
        exit(1);
    } else if ((new_size > 0) != 0) {
        if (new_size > old_size) {
            // Zero out newly allocated memory
            memset(new_ptr + old_size, 0, new_size - old_size);
        }
    }
    return new_ptr;
}

uint8_t get_opcode(const char* instruction, const uint8_t* conf, size_t confSize) {
    printf("%s\n", instruction);
    char *buffer = malloc(4 * sizeof(char));
    for(int i = 0; i < (int) confSize; i+=6) {
        memcpy(buffer, &(conf[i+1]), 3);
        buffer[3] = '\0';
        if (strcmp(instruction, buffer) == 0) {
            return conf[i];
        }
    }
    return false;
}

uint16_t get_operand(const char* operand) {
    // Operand
    printf("Operand: %s\n", operand);
    return (uint16_t) strtol(operand, NULL, 16);
}

void get_label(Labels **label_addresses, const char label[MAX_TOKEN_LEN + 1], const uint8_t current_token, uint8_t *current_size) {
    Labels *labels = calloc(1, sizeof(Labels));
    if (labels == NULL) {
        return;
    }
    if (*label_addresses == NULL) {
        *label_addresses = calloc(1, sizeof(Labels));
        if (*label_addresses == NULL) {
            free(labels);
            return;
        }
    } else {
        Labels *temp = realloc_zero(*label_addresses, (*current_size * sizeof(Labels)), (*current_size + 1) * sizeof(Labels));
        if (temp == NULL) {
            free(labels);
            return;
        }
        *label_addresses = temp;
    }
    strcpy(labels->label, label);
    //labels->address = (current_token == 0) ? 0 : (current_token - 1);
    labels->address = current_token;
    memcpy(&((*label_addresses)[*current_size]), labels, sizeof(Labels));
    (*current_size)++;
    free(labels);
}

// Function to lex the input string into a list of tokens
Token* lex(const char* input, uint8_t current_line) {
    // Allocate memory for the token array
    Token* tokens = calloc(1, sizeof(Token));
    int token_count = 0;

    // Read the input character by character
    int i = 0;
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
        if (isupper(input[i])) {
            // Allocate memory for the new token
            tokens = realloc_zero(tokens, token_count * sizeof(Token), sizeof(Token) * (token_count + 1));

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
            tokens = realloc_zero(tokens, token_count * sizeof(Token), sizeof(Token) * (token_count + 1));

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
        if (isxdigit(tolower(input[i]))) {
            // Allocate memory for the new token
            tokens = realloc_zero(tokens,  token_count * sizeof(Token), sizeof(Token) * (token_count + 1));

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
            tokens = realloc_zero(tokens, token_count * sizeof(Token), sizeof(Token) * (token_count + 1));
            int j = 0;
            while (isprint(input[i]) && j < MAX_TOKEN_LEN) {
                tokens[token_count].value[j] = input[i];
                i++;
                j++;
            }
            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = 4; // Set the token type to 4 for labels
            printf("Label %s\n", tokens[token_count].value);
            token_count++;
            continue;
        }

        // If none of the above cases are true, the input is invalid
        printf("Invalid input at line: %d\n", current_line);
        return NULL;
    }

    // Add a sentinel token to the end of the list
    tokens = realloc_zero(tokens, token_count * sizeof(Token), sizeof(Token) * (token_count + 1));
    tokens[token_count].value[0] = '\0';
    tokens[token_count].type = -1;

    return tokens;
}

// Function to parse the tokens into a list of instructions
void parse(Instruction *instructions, const Token *tokens, const uint8_t *current_token, Labels **label_addresses,
           size_t *current_size, const uint8_t *conf, size_t confSize) {
    if (tokens[0].type == TYPE_OPCODE) {

        // Get the opcode for the instruction or label
        instructions->opcode = get_opcode(tokens[0].value, conf, confSize);
        // Check for operands
        if (tokens[1].type == TYPE_REGISTER && tokens[2].type == TYPE_REGISTER && tokens[3].type == TYPE_OPERAND_2) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = get_operand(tokens[2].value);
            instructions->operand2 = get_operand(tokens[3].value);
        } else if (tokens[1].type == TYPE_REGISTER && tokens[2].type == TYPE_OPERAND_2) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = 0;
            instructions->operand2 = get_operand(tokens[2].value);
        } else if (tokens[1].type == TYPE_REGISTER && tokens[2].type == TYPE_REGISTER && tokens[3].type != TYPE_LABEL) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = get_operand(tokens[2].value);
            instructions->operand2 = 0;
        } else if (tokens[1].type == TYPE_REGISTER && tokens[2].type != TYPE_REGISTER) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = 0;
            instructions->operand2 = 0;
        } else if (tokens[1].type == TYPE_OPERAND_2) {
            instructions->operand2 = get_operand(tokens[1].value);
        } else if (tokens[1].type == TYPE_LABEL) {
            instructions->operand_rd = 0;
            instructions->operand_rn = 0;
            for(int i = 0; i < (int) *current_size; i++) {
                if(strcmp((*label_addresses)[i].label, tokens[1].value) == 0){
                    instructions->operand2 = (*label_addresses)[i].address;
                    return;
                }
            }
            fprintf(stderr, "Error: Label %s not found\n", tokens[1].value);
        } else if (tokens[1].type == TYPE_REGISTER && tokens[2].type == TYPE_REGISTER && tokens[3].type == TYPE_LABEL) {
            instructions->operand_rd = get_operand(tokens[1].value);
            instructions->operand_rn = get_operand(tokens[2].value);
            for(int i = 0; i < (int) *current_size; i++) {
                if(strcmp((*label_addresses)[i].label, tokens[3].value) == 0){
                    instructions->operand2 = (*label_addresses)[i].address;
                    return;
                }
            }
            fprintf(stderr, "Error: Label %s not found\n", tokens[3].value);
        } else {
            instructions->operand_rd = 0;
            instructions->operand_rn = 0;
            instructions->operand2 = 0;
        }
        return;
    } else if (tokens[0].type == TYPE_LABEL) {
        instructions->opcode = get_opcode("NOP", conf, confSize);
        instructions->operand_rd = 0;
        instructions->operand_rn = 0;
        instructions->operand2 = 0;
        get_label(label_addresses, (char *) tokens[0].value, *current_token, (uint8_t *) current_size);
    } else {
        printf("Invalid token\n");
        return;
    }
}

uint8_t *generate_code(const Instruction *instructions, uint8_t instruction_count, const uint8_t *conf, size_t confSize) {
    uint8_t *code = calloc(MAX_CODE_LENGTH, sizeof(uint8_t));
    uint8_t current_pointer = 0;

    for (uint8_t i = 0; i < instruction_count; i++) {
        const uint8_t opcode = instructions[i].opcode;
        uint8_t operand1;
        uint8_t operand2 = instructions[i].operand2;
        if (operand1_mode(opcode, conf, confSize) == 0) {
            operand1 = (instructions[i].operand_rd << 4) | instructions[i].operand_rn;
        } else {
            operand1 = ((uint16_t) instructions[i].operand2 & (uint16_t) 0xFF00) >> 8;
            operand2 = (uint16_t) instructions[i].operand2 & (uint16_t) 0x00FF;
        }
        const uint8_t num_ops = num_operands(opcode, conf, confSize);

        switch (num_ops) {
            case 0:
                code[current_pointer++] = opcode;
                break;
            case 1:
                code[current_pointer++] = opcode;
                code[current_pointer++] = operand1;
                break;
            case 2:
                code[current_pointer++] = opcode;
                code[current_pointer++] = operand1;
                code[current_pointer++] = operand2;
                break;
            case 3:
                code[current_pointer++] = opcode;
                code[current_pointer++] = operand1;
                code[current_pointer++] = (instructions[i].operand2 & 0xFF00) >> 8;
                code[current_pointer++] = (instructions[i].operand2 & 0x00FF);
                break;
            default:
                break;
        }
    }

    return code;
}

uint8_t num_operands(uint8_t opcode, const uint8_t *conf, size_t confSize) {
    uint8_t num_ops;
    for(int i = 0; i < (int) confSize; i+=6) {
        num_ops = conf[i+4];
        if(conf[i] == opcode) {
            return num_ops;
        }
    }
    return false;
}


uint8_t operand1_mode(uint8_t opcode, const uint8_t *conf, size_t confSize) {
    uint8_t operand1_mode;
    for (int i = 0; i < (int) confSize; i += 6) {
        operand1_mode = conf[i + 5];
        if (conf[i] == opcode) {
            return operand1_mode;
        }
    }
    return false;
}