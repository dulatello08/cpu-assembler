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

    // declare an array of strings to store the lines of the file
    char** lines = NULL;
    int num_lines = 0;

    // read each line of the file
    char line[10];
    while (fgets(line, 10, input_file) != NULL) {
        // allocate memory for the new string
        lines = realloc(lines, sizeof(char*) * (num_lines + 1));
        lines[num_lines] = malloc(sizeof(char) * (strlen(line) + 1));

        // copy the line into the array
        strcpy(lines[num_lines], line);
        num_lines++;
    }

    // Lex the input
    Token* tokens = malloc(sizeof(Token));
    int num_tokens = 0;
    for (int i = 0; i < num_lines; i++) {
        Token* line_tokens = lex(lines[i]);
        int num_line_tokens = 0;
        while (line_tokens[num_line_tokens].type != -1) {
            num_line_tokens++;
        }
        tokens = realloc(tokens, sizeof(Token) * (num_tokens + num_line_tokens + 1));
        for (int j = 0; j < num_line_tokens; j++) {
            tokens[num_tokens + j] = line_tokens[j];
        }
        num_tokens += num_line_tokens;
        free(line_tokens);
    }

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