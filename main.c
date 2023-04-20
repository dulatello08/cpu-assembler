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
    char* input_filename = NULL;
    char* conf_filename = NULL;
    char* output_filename = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (i + 1 < argc) {
                input_filename = argv[++i];
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--conf") == 0) {
            if (i + 1 < argc) {
                conf_filename = argv[++i];
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_filename = argv[++i];
            }
        } else {
            printf("Usage: assembler -i <input_file> -c <configuration_file> [-o <output_file>]\n");
            return 1;
        }
    }

    if (!input_filename || !conf_filename) {
        printf("Usage: assembler -i <input_file> -c <configuration_file> [-o <output_file>]\n");
        return 1;
    }

    if (!output_filename) {
        output_filename = "program.m";
    }

    // Process the input file
    FILE* input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 1;
    }

    // Process the conf file
    FILE* configuration_file = fopen(conf_filename, "r");
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