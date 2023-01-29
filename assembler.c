//
// Created by dulat on 1/1/23.
//
#include "main.h"

void *realloc_zero(void *ptr, size_t new_size) {
    void *new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL) {
        // Handle allocation error
        fprintf(stderr, "Error: Failed to reallocate memory\n");
        exit(1);
    } else if ((new_size > 0) != 0) {
        size_t old_size = malloc_usable_size(new_ptr);
        if (new_size > old_size) {
            // Zero out newly allocated memory
            memset(new_ptr + old_size, 0, new_size - old_size);
        }
    }
    return new_ptr;
}

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
    } else if (strcmp(instruction, "TSK") == 0) {
        return 0x19;
    } else if (strcmp(instruction, "SCH") == 0) {
        return 0x1A;
    } else if (strcmp(instruction, "SWT") == 0) {
        return 0x1B;
    } else if (strcmp(instruction, "KIL") == 0) {
        return 0x1C;
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
    // If the operand is not a register, it must be an immediate value
    return (uint16_t) strtol(operand, NULL, 16);
}

 void get_label(Labels **label_addresses, char label[16], uint8_t current_token, uint8_t *current_size) {
    *label_addresses = realloc_zero(*label_addresses, (*current_size+1) * sizeof(Labels));
    struct Labels *labels = calloc(1, sizeof(Labels));
    strcpy(labels->label, label);
    labels->address = current_token - 1;
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
            tokens = realloc_zero(tokens, sizeof(Token) * (token_count + 1));

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
            tokens = realloc_zero(tokens, sizeof(Token) * (token_count + 1));

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
            tokens = realloc_zero(tokens, sizeof(Token) * (token_count + 1));

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
            tokens = realloc_zero(tokens, sizeof(Token) * (token_count + 1));
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
    tokens = realloc_zero(tokens, sizeof(Token) * (token_count + 1));
    tokens[token_count].value[0] = '\0';
    tokens[token_count].type = -1;

    return tokens;
}

// Function to parse the tokens into a list of instructions
void parse(Instruction *instructions, const Token *tokens, uint8_t current_token, Labels **label_addresses,
           size_t *current_size) {
    if (tokens[0].type == TYPE_OPCODE) {

        // Get the opcode for the instruction or label
        instructions->opcode = get_opcode(tokens[0].value);
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
        instructions->opcode = get_opcode("NOP");
        instructions->operand_rd = 0;
        instructions->operand_rn = 0;
        instructions->operand2 = 0;
        get_label(label_addresses, (char *) tokens[0].value, current_token, (uint8_t *) current_size);
    } else {
        printf("Invalid token\n");
        return;
    }
}

// Function to generate machine code from the instructions
uint8_t * generate_code(Instruction* instructions, uint8_t instruction_count) {
    // Allocate memory for the machine code array
    uint8_t *code = malloc(sizeof(uint8_t) * 255);
    memset(code, 0, 255);
    uint8_t code_len = 0;
    uint8_t i = 0;
    // Generate the machine code for each instruction
    while (code_len < instruction_count) {
        // Get all operands and opcode
        uint8_t opcode = instructions[i].opcode;
        uint8_t operand1;
        if(operand1_mode(opcode)==0) {
            operand1 = (instructions[i].operand_rd << 4) | instructions[i].operand_rn;
        } else {
            operand1 = instructions[i].operand2;
        }
        uint8_t operand2 = instructions[i].operand2;
        // Write the instruction words to the machine code array
        switch (num_ops(instructions[i].opcode)) {
            case 0:
                code[code_len] = opcode;
                break;
            case 1:
                code[code_len] = opcode;
                code[code_len + 1] = operand1;
            case 2:
                code[code_len] = opcode;
                code[code_len + 1] = operand1;
                code[code_len + 2] = operand2;
        }
        code_len += num_ops(instructions[i].opcode);
    }

    // Return the machine code array
    return code;
}

uint8_t num_ops(uint8_t opcode) {
    switch (opcode) {
        case OP_NOP:
        case OP_POP:
        case OP_PRT:
        case OP_BRN:
        case OP_BRZ:
        case OP_BRO:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_STO:
        case OP_STM:
        case OP_LDM:
        case OP_PSH:
        case OP_CLZ:
            return 1;
        case OP_ADM:
        case OP_SBM:
        case OP_MLM:
        case OP_ADR:
        case OP_SBR:
        case OP_MLR:
        case OP_RDM:
        case OP_RNM:
        case OP_BRR:
        case OP_BNR:
            return 2;
        case OP_HLT:
        default:
            return 0;
    }
}

uint8_t operand1_mode(uint8_t opcode) {
    switch (opcode) {
        case OP_BRN:
        case OP_BRO:
        case OP_BRZ:
            return 1;
        default:
            return 0;
    }
}