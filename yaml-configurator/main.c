#include "main.h"

typedef struct {
    int opcode;
    int num_ops;
    int op1_mode;
    char *name;
} Opcode;

int main(int argc, char **argv) {
    int option;
    char *inputFileName = NULL;
    char *outputFileName = NULL;
    FILE *inputFile = NULL;
    FILE *outputFile = NULL;

    // Parse command line arguments
    static struct option long_options[] = {
            {"input", required_argument, 0, 'i'},
            {"output", required_argument, 0, 'o'},
            {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "i:o:", long_options, NULL)) != -1) {
        switch (option) {
            case 'i':
                inputFileName = optarg;
                break;
            case 'o':
                outputFileName = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-i input_file] [-o output_file]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    // Open input file
    if (inputFileName) {
        inputFile = fopen(inputFileName, "r");
        if (!inputFile) {
            fprintf(stderr, "Failed to open input file %s\n", inputFileName);
            exit(EXIT_FAILURE);
        }
    }

    // Open output file
    if (outputFileName) {
        outputFile = fopen(outputFileName, "w");
        if (!outputFile) {
            fprintf(stderr, "Failed to open output file %s\n", outputFileName);
            exit(EXIT_FAILURE);
        }
    }

    yaml_parser_t parser;
    yaml_event_t event;
    Opcode *opcodes = NULL;
    int num_opcodes = 0;
    int max_opcodes = 0;

    // Initialize parser
    if (!yaml_parser_initialize(&parser)) {
        printf("Failed to initialize parser\n");
        return 1;
    }

    // Set input file
    yaml_parser_set_input_file(&parser, inputFile);

    // Parse events
    while (1) {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("Failed to parse event\n");
            return 1;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            if (strcmp((char *)event.data.scalar.value, "opcode") == 0) {
                // Opcode scalar found, read next event for opcode value
                if (!yaml_parser_parse(&parser, &event)) {
                    printf("Failed to parse event\n");
                    return 1;
                }
                if (num_opcodes >= max_opcodes) {
                    // Resize opcodes array if necessary
                    max_opcodes = max_opcodes == 0 ? 1 : max_opcodes * 2;
                    opcodes = realloc(opcodes, max_opcodes * sizeof(Opcode));
                    if (!opcodes) {
                        printf("Failed to allocate memory\n");
                        return 1;
                    }
                }
                opcodes[num_opcodes].opcode = strtol((char *)event.data.scalar.value, NULL, 0);
            } else if (strcmp((char *)event.data.scalar.value, "name") == 0) {
                // Num_ops scalar found, read next event for num_ops value
                if (!yaml_parser_parse(&parser, &event)) {
                    printf("Failed to parse event\n");
                    return 1;
                }
                opcodes[num_opcodes].name = (char *)event.data.scalar.value;
                num_opcodes++;
            } else if (strcmp((char *)event.data.scalar.value, "num_ops") == 0) {
                // Num_ops scalar found, read next event for num_ops value
                if (!yaml_parser_parse(&parser, &event)) {
                    printf("Failed to parse event\n");
                    return 1;
                }
                opcodes[num_opcodes-1].num_ops = strtol((char *)event.data.scalar.value, NULL, 0);
            } else if (strcmp((char *)event.data.scalar.value, "op1_mode") == 0) {
                // Num_ops scalar found, read next event for num_ops value
                if (!yaml_parser_parse(&parser, &event)) {
                    printf("Failed to parse event\n");
                    return 1;
                }
                opcodes[num_opcodes-1].op1_mode = strtol((char *)event.data.scalar.value, NULL, 0);
            }
        }

        if (event.type == YAML_STREAM_END_EVENT) {
            // End of stream, break out of loop
            break;
        }

        // Free event
        yaml_event_delete(&event);
    }

    // Print opcodes
    uint8_t *outputArray = malloc(num_opcodes*3 * sizeof(uint8_t));
    for (int i = 0; i < num_opcodes; i++) {
        printf("Opcode: 0x%02X, Num_ops: %d, Op2_mode: %d\n", opcodes[i].opcode, opcodes[i].num_ops, opcodes[i].op1_mode);
        outputArray[i] = opcodes[i].opcode;
        outputArray[i+1] = opcodes[i].name[0];
        outputArray[i+2] = opcodes[i].name[1];
        outputArray[i+3] = opcodes[i].name[2];
        outputArray[i+5] = opcodes[i].num_ops;
        outputArray[i+6] = opcodes[i].op1_mode;
    }
    fwrite(outputArray, sizeof(uint8_t), num_opcodes*3, outputFile);
    free(outputArray);

    // Cleanup
    yaml_parser_delete(&parser);
    fclose(inputFile);
    fclose(outputFile);
    free(opcodes);
    return 0;
}