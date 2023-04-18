//
// Created by dulat on 1/1/23.
//

#include "main.h"

// Function to write the machine code to a file
void write_code(uint8_t *code, uint16_t code_len, const char* filename) {
    // Open the file for writing
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    for (int i = 0; i < code_len; i++) {
        printf("Code[%d]: %02hx\n", i, code[i]);
    }
    // Write the machine code to the file
    size_t bytes_written = fwrite(code, sizeof(uint8_t), code_len, file);
    if (bytes_written != (size_t)code_len) {
        perror("Error writing to file");
    }

    // Close the file
    fclose(file);
}


int main(int argc, char* argv[]) {
    // Check for correct number of arguments
    if (argc != 4 && argc != 3) {
        printf("Usage: assembler <input_file> <configuration_file> [output_file]\n");
        return 1;
    }

    // Open the input file
    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 1;
    }

    // Open the conf file
    FILE* configuration_file = fopen(argv[2], "r");
    if (configuration_file == NULL) {
        printf("Error opening configuration file\n");
        return 1;
    }

    // Get the file size
    fseek(configuration_file, 0, SEEK_END);
    long file_size = ftell(configuration_file);
    fseek(configuration_file, 0, SEEK_SET);

    // Allocate memory for the data
    uint8_t* conf = malloc(file_size);
    if (conf == NULL) {
        printf("Error allocating memory\n");
        return 1;
    }

    // Read the data into memory
    size_t bytes_read = fread(conf, sizeof(uint8_t), file_size, configuration_file);
    if (bytes_read != (size_t) file_size) {
        printf("Error reading file\n");
        free(conf);
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
        tokens = realloc_zero(tokens, sizeof(Token)*(tokenLen+1));
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
    Labels *labels = calloc(1, sizeof(Labels));
    size_t current_size = 1;
    uint16_t instruction_count = 0;
    uint8_t current_token = 0;
    while (instruction_count < tokenLen) {
        parse(&instructions[instruction_count], tokens[instruction_count], &current_token, &labels, &current_size, conf, bytes_read);
        current_token += num_operands(instructions[instruction_count].opcode, conf, bytes_read) + 1;
        instruction_count++;
    }
    // Generate machine code

    uint8_t *code = generate_code(instructions, instruction_count, conf, bytes_read);
    //uint16_t* code_len = get_code_len(instructions);

    // Write the machine code to the output file
    if (argc == 4) {
        // Use the output file specified by the user
        write_code(code, PROGRAM_WORDS, argv[4]);
    } else {
        // Use the default output file
        write_code(code, PROGRAM_WORDS, "program.m");
    }
    free(code);
    free(lines);
    // Clean up
    free(tokens);
    free(instructions);
    free(labels);
    return 0;
}