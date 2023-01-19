//
// Created by dulat on 1/1/23.
//

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// Function to write the machine code to a file
void write_code(uint16_t* code, int16_t code_len, const char* filename) {
    // Open the file for writing
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    *code = htons(*code);
    for (int i = 0; i < code_len; i++) {
        printf("Code[%d]: %04hx\n", i, code[i]);
    }
    // Write the machine code to the file
    size_t bytes_written = fwrite(code, sizeof(uint16_t), code_len, file);
    if (bytes_written != (size_t)code_len) {
        perror("Error writing to file");
    }

    // Close the file
    fclose(file);
}

// Function to get the length of the machine code array
/*uint16_t* get_code_len(Instruction** instructions) {
    uint16_t* code_len = malloc(sizeof(uint16_t));
    int i, j = 0;
    while (i<(int)sizeof(*instructions)/(int)sizeof(instructions[0])) {
        while (instructions[i][j].opcode != SENTINEL_VALUE) {
            code_len = realloc(code_len, sizeof(uint16_t)*(i+1));
            code_len[i]++;
            j++;
        }
        i++;
    }
    return code_len;
}
*/

void* realloc_zero(void* pBuffer, size_t oldSize, size_t newSize) {
  void* pNew = realloc(pBuffer, newSize);
  if ( newSize > oldSize && pNew ) {
    size_t diff = newSize - oldSize;
    void* pStart = ((char*)pNew) + oldSize;
    memset(pStart, 0, diff);
  }
  return pNew;
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
    char line[1024];
    while (fgets(line, 1024, input_file) != NULL) {
        // allocate memory for the new string
        lines = realloc(lines, sizeof(char*) * (num_lines + 1));
        lines[num_lines] = malloc(sizeof(char) * (strlen(line) + 1));

        // copy the line into the array
        strcpy(lines[num_lines], line);
        num_lines++;
    }

    // Lex the input
    Token** tokens = calloc(1, sizeof(Token));
    int tokenLen = 0;
    for (int i = 0; i < num_lines; i++) {
        tokens = realloc_zero(tokens, sizeof(Token)*(tokenLen), sizeof(Token)*(tokenLen+1));
        tokens[i] = lex(lines[i], i);
        tokenLen++;
    }

    // Parse the tokens
    Instruction *instructions;
    instructions = calloc(tokenLen, sizeof(Instruction));
    if (instructions == NULL) {
        // allocation failed
        return -1;
    }
    uint16_t instruction_count = 0;
    Labels *labels = malloc(sizeof(Labels));
    size_t current_size = 0;
    while (instruction_count < tokenLen) {
        parse(&instructions[instruction_count], tokens[instruction_count], instruction_count, labels, &current_size);
        instruction_count++;
    }
    // Generate machine code

    uint16_t* code = generate_code(instructions, instruction_count);
    //uint16_t* code_len = get_code_len(instructions);

    // Write the machine code to the output file
    if (argc == 3) {
        // Use the output file specified by the user
        write_code(code, 255, argv[2]);
    } else {
        // Use the default output file
        write_code(code, 255, "program.m");
    }
    free(code);
    free(lines);
    // Clean up
    free(tokens);
    free(instructions);

    return 0;
}