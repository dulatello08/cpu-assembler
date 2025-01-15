/*
 * Example C program that:
 * 1) Uses getopt to accept -i (input YAML) and -o (output BIN).
 * 2) Parses the YAML file using libyaml.
 * 3) Writes a compact binary file representing instructions.
 *
 * Compile with:
 *     cc -o parser parser.c -lyaml
 */

#include "main.h"

// ---------------------------------------------------------------------
// DATA STRUCTURES
// ---------------------------------------------------------------------

typedef struct {
    uint8_t mode;
    uint8_t length;
} Specifier;

typedef struct {
    char    name[32];   // e.g., "rd", "rn", "operand_2", ...
    uint32_t width;
} Operand;

typedef struct {
    uint8_t     opcode;
    char        name[32];
    Specifier  *specifiers;
    size_t      numSpecifiers;
    Operand    *operands;
    size_t      numOperands;
} Instruction;

// A container to hold multiple instructions
typedef struct {
    Instruction *instructions;
    size_t       count;
} InstructionSet;

// ---------------------------------------------------------------------
// FUNCTION PROTOTYPES
// ---------------------------------------------------------------------
static void usage(const char *progname);
static InstructionSet parse_yaml_file(const char *filename);
static void write_binary_file(const char *filename, const InstructionSet *iset);
static void free_instruction_set(InstructionSet *iset);

// ---------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------
int main(int argc, char **argv)
{
    int opt;
    char *inputFile = NULL;
    char *outputFile = NULL;

    // Parse command line options
    while ((opt = getopt(argc, argv, "i:o:h")) != -1) {
        switch (opt) {
        case 'i':
            inputFile = optarg;
            break;
        case 'o':
            outputFile = optarg;
            break;
        case 'h':
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if (!inputFile || !outputFile) {
        usage(argv[0]);
        return 1;
    }

    // Parse YAML
    InstructionSet iset = parse_yaml_file(inputFile);

    // Write out the binary
    write_binary_file(outputFile, &iset);

    // Clean up
    free_instruction_set(&iset);

    return 0;
}

// ---------------------------------------------------------------------
// USAGE
// ---------------------------------------------------------------------
static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s -i <input YAML> -o <output BIN>\n", progname);
}

// ---------------------------------------------------------------------
// PARSE YAML
// ---------------------------------------------------------------------

/*
 * This function reads a sequence of YAML documents in the form:
 *
 * - opcode: 0x00
 *   name: "nop"
 *   specifiers: []
 *   operands: []
 *   length: 2
 *
 * and so forth.  It returns an InstructionSet containing all parsed instructions.
 *
 * Notes:
 *  - The parsing logic is simplified. You may want better error checking in production code.
 *  - We assume the top-level structure is a YAML sequence of mappings.
 */
static InstructionSet parse_yaml_file(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open input file");
        exit(1);
    }

    yaml_parser_t parser;
    yaml_event_t  event;

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize YAML parser\n");
        fclose(fp);
        exit(1);
    }
    yaml_parser_set_input_file(&parser, fp);

    // Our result data
    InstructionSet iset;
    iset.instructions = NULL;
    iset.count = 0;

    // We'll track whether we're currently inside a list item (one instruction).
    Instruction currentInstruction;
    memset(&currentInstruction, 0, sizeof(currentInstruction));

    // Temporary storage for specifiers / operands
    Specifier *tempSpecifiers = NULL;
    size_t     specCount       = 0;
    Operand   *tempOperands   = NULL;
    size_t     operandCount    = 0;

    enum {
        STATE_START,
        STATE_IN_SEQUENCE,
        STATE_IN_INSTRUCTION,    // processing a mapping of one instruction
        STATE_IN_SPECIFIERS,
        STATE_IN_OPERANDS
    } state = STATE_START;

    // Helper flags or trackers for which key we found
    char currentKey[128] = {0};

    // We use a small stack approach for parsing (since it's relatively simple data)
    while (1) {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "YAML parse error: %s\n", parser.problem);
            goto parse_error;
        }

        switch (event.type) {

        case YAML_STREAM_START_EVENT:
            // Start of document
            break;

        case YAML_STREAM_END_EVENT:
            // End of the entire YAML
            goto done;  

        case YAML_DOCUMENT_START_EVENT:
            // Start of a doc (we ignore docs vs. sequences in this example)
            break;

        case YAML_DOCUMENT_END_EVENT:
            // End of a doc
            break;

        case YAML_SEQUENCE_START_EVENT:
            if (state == STATE_START) {
                // We found the top-level sequence
                state = STATE_IN_SEQUENCE;
            }
            else if (strcmp(currentKey, "specifiers") == 0) {
                // We are going into the specifiers array
                state = STATE_IN_SPECIFIERS;
                // Prepare
                specCount = 0;
                free(tempSpecifiers);
                tempSpecifiers = NULL;
            }
            else if (strcmp(currentKey, "operands") == 0) {
                // We are going into the operands array
                state = STATE_IN_OPERANDS;
                // Prepare
                operandCount = 0;
                free(tempOperands);
                tempOperands = NULL;
            }
            break;

        case YAML_SEQUENCE_END_EVENT:
            if (state == STATE_IN_SPECIFIERS) {
                // Attach specifiers to the instruction
                currentInstruction.specifiers    = tempSpecifiers;
                currentInstruction.numSpecifiers = specCount;
                tempSpecifiers = NULL;
                specCount = 0;
                state = STATE_IN_INSTRUCTION;
            }
            else if (state == STATE_IN_OPERANDS) {
                // Attach operands
                currentInstruction.operands    = tempOperands;
                currentInstruction.numOperands = operandCount;
                tempOperands = NULL;
                operandCount = 0;
                state = STATE_IN_INSTRUCTION;
            }
            else if (state == STATE_IN_SEQUENCE) {
                // This means we've reached the end of the top-level sequence (?), 
                // but let's see if there's more to parse in the doc.
                // We'll do nothing special here. We might rely on YAML_STREAM_END_EVENT or DOC_END.
            }
            break;

        case YAML_MAPPING_START_EVENT:
            if (state == STATE_IN_SEQUENCE) {
                // Starting a new instruction
                state = STATE_IN_INSTRUCTION;
                memset(&currentInstruction, 0, sizeof(currentInstruction));
            }
            else if (state == STATE_IN_SPECIFIERS) {
                // We have a new specifier object
                tempSpecifiers = realloc(tempSpecifiers, (specCount + 1)*sizeof(Specifier));
                memset(&tempSpecifiers[specCount], 0, sizeof(Specifier));
            }
            else if (state == STATE_IN_OPERANDS) {
                // We have a new operand object
                tempOperands = realloc(tempOperands, (operandCount + 1)*sizeof(Operand));
                memset(&tempOperands[operandCount].name, 0, sizeof(tempOperands[operandCount].name));
            }
            break;

        case YAML_MAPPING_END_EVENT:
            if (state == STATE_IN_SPECIFIERS) {
                // Done with one specifier
                specCount++;
            }
            else if (state == STATE_IN_OPERANDS) {
                operandCount++;
            }
            else if (state == STATE_IN_INSTRUCTION) {
                // We finished reading one instruction
                // Save currentInstruction into the set
                iset.instructions = realloc(iset.instructions, (iset.count + 1)*sizeof(Instruction));
                iset.instructions[iset.count] = currentInstruction;
                iset.count++;

                // Reset currentInstruction to safe state
                memset(&currentInstruction, 0, sizeof(currentInstruction));
                currentInstruction.specifiers = NULL;
                currentInstruction.operands   = NULL;
                state = STATE_IN_SEQUENCE;
            }
            break;

        case YAML_SCALAR_EVENT: {
            // We have some scalar. Depending on state, interpret it
            const char *val = (const char *)event.data.scalar.value;

            if (event.data.scalar.style == YAML_SINGLE_QUOTED_SCALAR_STYLE ||
                event.data.scalar.style == YAML_DOUBLE_QUOTED_SCALAR_STYLE ||
                event.data.scalar.style == YAML_PLAIN_SCALAR_STYLE) {

                // If it is a key in a mapping, we store it
                static int expectingValue = 0;
                if (!expectingValue) {
                    // This is a key
                    strncpy(currentKey, val, sizeof(currentKey)-1);
                    currentKey[sizeof(currentKey)-1] = '\0';
                    expectingValue = 1;
                } else {
                    // This is the value
                    // Interpret based on currentKey and state
                    if (state == STATE_IN_INSTRUCTION) {
                        if (strcmp(currentKey, "opcode") == 0) {
                            // opcode: 0x01 or something
                            unsigned int temp;
                            if (sscanf(val, "%x", &temp) == 1) {
                                currentInstruction.opcode = (uint8_t)temp;
                            }
                        }
                        else if (strcmp(currentKey, "name") == 0) {
                            strncpy(currentInstruction.name, val, sizeof(currentInstruction.name)-1);
                        }
                        // length might be present for certain instructions (some example had 'length: 2')
                        // But from your structure we are not storing it. If you want to store it in the future, do it here:
                        // else if (strcmp(currentKey, "length") == 0) { ... }
                    }
                    else if (state == STATE_IN_SPECIFIERS) {
                        // We'll fill in the last specifier
                        if (strcmp(currentKey, "mode") == 0) {
                            unsigned int temp;
                            if (sscanf(val, "%x", &temp) == 1) {
                                tempSpecifiers[specCount].mode = (uint8_t)temp;
                            }
                        } else if (strcmp(currentKey, "length") == 0) {
                            unsigned int temp;
                            if (sscanf(val, "%u", &temp) == 1) {
                                tempSpecifiers[specCount].length = (uint8_t)temp;
                            }
                        }
                    }
                    else if (state == STATE_IN_OPERANDS) {
                        // We'll fill in the last operand
                        // The key is the operand's name (like "rd", "rn", "operand_2"), 
                        // but the value is a nested mapping "width: 8"
                        // Actually, from the example:
                        //    - rd:
                        //        width: 8
                        // This means the top-level scalar is the name "rd:", then a mapping with "width". 
                        // That requires deeper logic. However, the example YAML has each operand as:
                        //    - rd:
                        //        width: 8
                        // i.e. we first get the key "rd" and then inside the mapping we get "width" -> "8".
                        // But the sample code lumps them into the same bracket. 
                        // For simplicity, let's interpret the YAML as if "rd" is the key, "width" is another key. 
                        // That actually requires a more advanced nested parse. 
                        // Because the example is:
                        //   - operand_2:
                        //       width: 16
                        // So we see the operand name "operand_2" THEN a nested key "width".

                        // We'll do something simpler: if the currentKey is "rd:", we store "rd" as the operand name.
                        // Then if the next key is "width", we read the value. 
                        // This code is a simplified approach. 
                        if (strcmp(currentKey, "width") == 0) {
                            // store width in last operand
                            unsigned int temp;
                            if (sscanf(val, "%u", &temp) == 1) {
                                tempOperands[operandCount].width = (uint32_t)temp;
                            }
                        } else {
                            // This is presumably the operand name
                            // We store it (minus any trailing colon if present)
                            char operandName[32];
                            strncpy(operandName, currentKey, sizeof(operandName)-1);
                            operandName[sizeof(operandName)-1] = '\0';
                            // remove trailing colon if present
                            char *colon = strchr(operandName, ':');
                            if (colon) *colon = '\0';
                            strncpy(tempOperands[operandCount].name, operandName, sizeof(tempOperands[operandCount].name)-1);
                            // Now store the value if it isn't "width" but something else
                            // (But in the sample, the next scalar after "rd:" is actually another mapping start. 
                            //  So we'll not do anything here.)
                        }
                    }
                    expectingValue = 0; // reset
                }
            }
            break;
        }

        default:
            // We ignore other events in this simplified parser
            break;
        }

        yaml_event_delete(&event);
    }

done:
    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(fp);
    return iset;

parse_error:
    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(fp);
    exit(1);
}

// ---------------------------------------------------------------------
// WRITE BINARY
// ---------------------------------------------------------------------
/*
 * Write a compact binary format that contains:
 *
 *  1) A 32-bit count of how many instructions
 *  For each instruction:
 *     - opcode (1 byte)
 *     - name (fixed 32 bytes, zero-padded)
 *     - numSpecifiers (4 bytes)
 *        For each specifier:
 *          - mode (1 byte)
 *          - length (1 byte)
 *     - numOperands (4 bytes)
 *        For each operand:
 *          - name (fixed 32 bytes, zero-padded)
 *          - width (4 bytes)
 *
 * You can tailor this format as desired.
 */
static void write_binary_file(const char *filename, const InstructionSet *iset)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to open output file");
        exit(1);
    }

    // 1) Write instruction count (32-bit)
    uint32_t count = (uint32_t)iset->count;
    fwrite(&count, sizeof(uint32_t), 1, fp);

    // 2) For each instruction
    for (size_t i = 0; i < iset->count; i++) {
        const Instruction *in = &iset->instructions[i];

        // opcode
        fwrite(&in->opcode, sizeof(uint8_t), 1, fp);

        // name (32 bytes, zero-padded)
        char nameBuf[32];
        memset(nameBuf, 0, sizeof(nameBuf));
        strncpy(nameBuf, in->name, sizeof(nameBuf)-1);
        fwrite(nameBuf, sizeof(nameBuf), 1, fp);

        // numSpecifiers (4 bytes)
        uint32_t sCount = (uint32_t)in->numSpecifiers;
        fwrite(&sCount, sizeof(uint32_t), 1, fp);

        // each specifier: mode (1 byte) + length (1 byte)
        for (size_t s = 0; s < in->numSpecifiers; s++) {
            fwrite(&in->specifiers[s].mode,   sizeof(uint8_t), 1, fp);
            fwrite(&in->specifiers[s].length, sizeof(uint8_t), 1, fp);
        }

        // numOperands (4 bytes)
        uint32_t oCount = (uint32_t)in->numOperands;
        fwrite(&oCount, sizeof(uint32_t), 1, fp);

        // each operand: name (32 bytes, zero-padded) + width (4 bytes)
        for (size_t o = 0; o < in->numOperands; o++) {
            char opNameBuf[32];
            memset(opNameBuf, 0, sizeof(opNameBuf));
            strncpy(opNameBuf, in->operands[o].name, sizeof(opNameBuf)-1);
            fwrite(opNameBuf, sizeof(opNameBuf), 1, fp);
            fwrite(&in->operands[o].width, sizeof(uint32_t), 1, fp);
        }
    }

    fclose(fp);
}

// ---------------------------------------------------------------------
// FREE INSTRUCTION SET
// ---------------------------------------------------------------------
static void free_instruction_set(InstructionSet *iset)
{
    if (!iset->instructions) return;

    for (size_t i = 0; i < iset->count; i++) {
        free(iset->instructions[i].specifiers);
        free(iset->instructions[i].operands);
    }
    free(iset->instructions);
    iset->instructions = NULL;
    iset->count = 0;
}