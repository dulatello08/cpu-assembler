#include "main.h"

typedef struct {
    int opcode;
    int num_ops;
    int op2_mode;
} Opcode;

int main() {
    FILE *file = fopen("test.yaml", "r");  // replace with your YAML file name
    if (!file) {
        printf("Failed to open file\n");
        return 1;
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
    yaml_parser_set_input_file(&parser, file);

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
            } else if (strcmp((char *)event.data.scalar.value, "num_ops") == 0) {
                // Num_ops scalar found, read next event for num_ops value
                if (!yaml_parser_parse(&parser, &event)) {
                    printf("Failed to parse event\n");
                    return 1;
                }
                opcodes[num_opcodes].num_ops = strtol((char *)event.data.scalar.value, NULL, 0);
                num_opcodes++;
            } else if (strcmp((char *)event.data.scalar.value, "op2_mode") == 0) {
                // Num_ops scalar found, read next event for num_ops value
                if (!yaml_parser_parse(&parser, &event)) {
                    printf("Failed to parse event\n");
                    return 1;
                }
                opcodes[num_opcodes].op2_mode = strtol((char *)event.data.scalar.value, NULL, 0);
                num_opcodes++;
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
    uint8_t *outputArray = malloc(3 * sizeof(uint8_t));
    for (int i = 0; i < num_opcodes; i++) {
        printf("Opcode: 0x%02X, Num_ops: %d, Op2_mode: %d\n", opcodes[i].opcode, opcodes[i].num_ops, opcodes[i].op2_mode);
        outputArray[i] = opcodes[i].opcode;
        outputArray[i+1] = opcodes[i].num_ops;
        outputArray[i+2] = opcodes[i].op2_mode;
        outputArray = realloc(outputArray, i*3);
    }

    // Cleanup
    yaml_parser_delete(&parser);
    fclose(file);
    free(opcodes);
    return 0;
}