//
// Created by dulat on 1/1/23.
//

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>

// Function to write the machine code to a file
void write_code(uint16_t* code, int16_t code_len, const char* filename) {
    // Open the file for writing
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    *code = htobe16(*code);
    // Write the machine code to the file
    size_t bytes_written = fwrite(code, sizeof(uint16_t), code_len, file);
    if (bytes_written != (size_t)code_len) {
        perror("Error writing to file");
    }

    // Close the file
    fclose(file);
}

// Function to get the length of the machine code array
int16_t get_code_len(Instruction* instructions) {
    int16_t code_len = 0;
    int i = 0;
    while (instructions[i].opcode != SENTINEL_VALUE) {
        code_len++;
        i++;
    }
    return code_len;
}

int main(int argc, char* argv[]) {
    // Check for correct number of arguments
    if (argc != 3 && argc != 2) {
        printf("Usage: assembler <input_file> [output_file]\n");
        return 1;
    }

    // Open the input file
    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 1;
    }

    // Read the input file into a string
    char input[1024];
    fgets(input, 1024, input_file);

    // Lex the input
    Token* tokens = lex(input);

    // Parse the tokens
    Instruction* instructions = parse(tokens);
    // Generate machine code

    uint16_t* code = generate_code(instructions);
    int16_t code_len = get_code_len(instructions);

    // Write the machine code to the output file
    if (argc == 3) {
        // Use the output file specified by the user
        write_code(code, code_len, argv[2]);
    } else {
        // Use the default output file
        write_code(code, code_len, "program.m");
    }
    free(code);

    // Clean up
    free(tokens);
    free(instructions);

    return 0;
}