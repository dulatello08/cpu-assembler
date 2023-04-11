#include <stdio.h>
#include <yaml.h>

typedef struct {
    int opcode;
    int num_ops;
} Opcode;

int main() {
    FILE *file = fopen("test.yaml", "r");  // replace with your YAML file name
    if (!file) {
        printf("Failed to open file\n");
        return 1;
    }

    yaml_parser_t parser;
    yaml_event_t event;
    Opcode opcode;

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
                opcode.opcode = strtol((char *)event.data.scalar.value, NULL, 0);
            } else if (strcmp((char *)event.data.scalar.value, "num_ops") == 0) {
                // Num_ops scalar found, read next event for num_ops value
                if (!yaml_parser_parse(&parser, &event)) {
                    printf("Failed to parse event\n");
                    return 1;
                }
                opcode.num_ops = strtol((char *)event.data.scalar.value, NULL, 0);
            }
        }

        if (event.type == YAML_SEQUENCE_END_EVENT) {
            // End of sequence, do something with opcode
            printf("Opcode: 0x%02X, Num_ops: %d\n", opcode.opcode, opcode.num_ops);
			opcode.opcode = 0;
            opcode.num_ops = 0;
        }

        if (event.type == YAML_STREAM_END_EVENT) {
            // End of stream, break out of loop
            break;
        }

        // Free event
        yaml_event_delete(&event);
    }

    // Cleanup
    yaml_parser_delete(&parser);
    fclose(file);
    return 0;
}