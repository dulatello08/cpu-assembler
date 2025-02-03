#!/usr/bin/env python3
import re

class Specifier:
    def __init__(self, sp, syntax, encoding, length):
        self.sp = sp
        self.syntax = syntax
        self.encoding = encoding
        self.length = length

class Instruction:
    def __init__(self, name, opcode):
        self.name = name
        self.opcode = opcode
        self.specifiers = []

def parse_md_file(filename):
    instructions = []
    current_instruction = None
    current_specifier = None
    in_specifiers = False

    with open(filename, 'r') as f:
        for line in f:
            stripped = line.strip()
            if not stripped or stripped.startswith("#"):
                continue  # Skip empty lines or comments

            if stripped.startswith("instruction"):
                # Start a new instruction
                _, name = stripped.split(None, 1)
                current_instruction = Instruction(name, None)
                instructions.append(current_instruction)
                in_specifiers = False

            elif stripped.startswith("opcode") and current_instruction:
                _, opcode_str = stripped.split()
                # Use base 0 so 0x prefix is recognized for opcode if needed
                current_instruction.opcode = int(opcode_str, 0)

            elif stripped.startswith("specifiers"):
                in_specifiers = True

            elif in_specifiers and current_instruction:
                # Within specifiers block
                if stripped.startswith("sp"):
                    parts = stripped.split()
                    if len(parts) >= 2:
                        sp_val = parts[1]
                        # Convert sp value using base 16
                        current_specifier = Specifier(int(sp_val, 16), None, None, None)
                        current_instruction.specifiers.append(current_specifier)
                elif stripped.startswith("syntax") and current_specifier:
                    match = re.search(r'syntax\s+"(.*?)"', stripped)
                    if match:
                        current_specifier.syntax = match.group(1)
                elif stripped.startswith("encoding") and current_specifier:
                    # Everything after 'encoding' is taken as the encoding string
                    encoding_str = stripped[len("encoding"):].strip()
                    current_specifier.encoding = encoding_str
                elif stripped.startswith("length") and current_specifier:
                    _, length_val = stripped.split()
                    current_specifier.length = int(length_val)

    return instructions

def generate_header(instructions, output_filename):
    with open(output_filename, 'w') as f:
        f.write("// Auto-generated instructions header\n")
        f.write("#ifndef INSTRUCTIONS_H\n#define INSTRUCTIONS_H\n\n")
        f.write("#include <cstdint>\n")
        f.write("#include <cstddef>\n\n")

        f.write("struct InstructionSpecifier {\n")
        f.write("    uint8_t sp;\n")
        f.write("    const char* syntax;\n")
        f.write("    const char* encoding;\n")
        f.write("    uint8_t length;\n")
        f.write("};\n\n")

        f.write("struct InstructionFormat {\n")
        f.write("    const char* name;\n")
        f.write("    uint8_t opcode;\n")
        f.write("    size_t num_specifiers;\n")
        f.write("    const InstructionSpecifier* specifiers;\n")
        f.write("};\n\n")

        # Generate specifier arrays for each instruction
        for inst in instructions:
            f.write(f"static const InstructionSpecifier {inst.name}_specs[] = {{\n")
            for spec in inst.specifiers:
                syntax = spec.syntax.replace('"', '\\"') if spec.syntax else ""
                encoding = spec.encoding.replace('"', '\\"') if spec.encoding else ""
                f.write(f"    {{{spec.sp}, \"{syntax}\", \"{encoding}\", {spec.length}}},\n")
            f.write("};\n\n")

        # Generate instructions array
        f.write("static const InstructionFormat instructions[] = {\n")
        for inst in instructions:
            f.write(f"    {{\"{inst.name}\", 0x{inst.opcode:02X}, {len(inst.specifiers)}, {inst.name}_specs}},\n")
        f.write("};\n\n")

        f.write("#endif // INSTRUCTIONS_H\n")

def main():
    import sys
    # Use command-line argument for MD file if provided
    md_file = sys.argv[1] if len(sys.argv) > 1 else "config/neocore16x32.mdesc"
    output_header = "assembler/machine_description.h"

    instructions = parse_md_file(md_file)
    generate_header(instructions, output_header)
    print(f"Generated {output_header} from {md_file}")

if __name__ == "__main__":
    main()
