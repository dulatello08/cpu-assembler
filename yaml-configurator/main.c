#include "main.h"

typedef struct {
    int opcode;
    int num_ops;
    int op1_mode;
    char *name;
} Opcode;

// Function to parse a single event and return the value
static int parse_yaml_event(yaml_parser_t *parser, yaml_event_t *event) {
    if (!yaml_parser_parse(parser, event)) {
        printf("Failed to parse event\n");
        return 0;
    }
    return 1;
}

// Function to parse and handle scalar events
static int handle_scalar_event(yaml_parser_t *parser, yaml_event_t *event, Opcode *opcodes, int *num_opcodes) {
    if (strcmp((char *)event->data.scalar.value, "opcode") == 0) {
        if (!parse_yaml_event(parser, event)) return 0;
        opcodes[*num_opcodes].opcode = strtol((char *)event->data.scalar.value, NULL, 0);
    } else if (strcmp((char *)event->data.scalar.value, "name") == 0) {
        if (!parse_yaml_event(parser, event)) return 0;
        opcodes[*num_opcodes].name = strdup((char *)event->data.scalar.value);
    } else if (strcmp((char *)event->data.scalar.value, "num_ops") == 0) {
        if (!parse_yaml_event(parser, event)) return 0;
        opcodes[*num_opcodes].num_ops = strtol((char *)event->data.scalar.value, NULL, 0);
        (*num_opcodes)++;
    } else if (strcmp((char *)event->data.scalar.value, "op1_mode") == 0) {
        if (!parse_yaml_event(parser, event)) return 0;
        opcodes[*(num_opcodes)-1].op1_mode = strtol((char *)event->data.scalar.value, NULL, 0);
    }
    return 1;
}

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

    if (!yaml_parser_initialize(&parser)) {
        printf("Failed to initialize parser\n");
        return 1;
    }

    yaml_parser_set_input_file(&parser, inputFile);

    while (1) {
        if (!parse_yaml_event(&parser, &event)) return 1;

        if (event.type == YAML_SCALAR_EVENT) {
            if (num_opcodes >= max_opcodes) {
                max_opcodes = max_opcodes == 0 ? 1 : max_opcodes * 2;
                opcodes = realloc(opcodes, max_opcodes * sizeof(Opcode));
                if (!opcodes) {
                    printf("Failed to allocate memory\n");
                    return 1;
                }
            }
            if (!handle_scalar_event(&parser, &event, opcodes, &num_opcodes)) return 1;
        }

        if (event.type == YAML_STREAM_END_EVENT) break;

        yaml_event_delete(&event);
    }

    // Print opcodes and write to output file
    uint8_t *outputArray = malloc(num_opcodes * 7 * sizeof(uint8_t));
    for (int i = 0; i < num_opcodes; i++) {
        printf("%s: 0x%02X, Num_ops: %d, Op1_mode: %d\n", opcodes[i].name, opcodes[i].opcode, opcodes[i].num_ops, opcodes[i].op1_mode);
        outputArray[i * 7] = opcodes[i].opcode;
        outputArray[i * 7 + 1] = opcodes[i].name[0];
        outputArray[i * 7 + 2] = opcodes[i].name[1];
        outputArray[i * 7 + 3] = opcodes[i].name[2];
        outputArray[i * 7 + 4] = opcodes[i].name[3];
        outputArray[i * 7 + 5] = opcodes[i].num_ops;
        outputArray[i * 7 + 6] = opcodes[i].op1_mode;
    }
    fwrite(outputArray, sizeof(uint8_t), num_opcodes * 7, outputFile);
    free(outputArray);

    // Cleanup
    yaml_parser_delete(&parser);
    fclose(inputFile);
    fclose(outputFile);
    for (int i = 0; i < num_opcodes; i++) {
        free(opcodes[i].name);
    }
    free(opcodes);
    return 0;
}