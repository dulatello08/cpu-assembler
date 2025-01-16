// Auto-generated instructions header
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <cstdint>
#include <cstddef>

struct InstructionSpecifier {
    uint8_t sp;
    const char* syntax;
    const char* encoding;
    uint8_t length;
};

struct InstructionFormat {
    const char* name;
    uint8_t opcode;
    size_t num_specifiers;
    const InstructionSpecifier* specifiers;
};

static const InstructionSpecifier nop_specs[] = {
    {0, "nop", "[sp(8)] [opcode(8)]", 1},
};

static const InstructionSpecifier add_specs[] = {
    {0, "add %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "add %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "add %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier sub_specs[] = {
    {0, "sub %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "sub %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "sub %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier mul_specs[] = {
    {0, "mul %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "mul %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "mul %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier and_specs[] = {
    {0, "and %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "and %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "and %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier or_specs[] = {
    {0, "or %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "or %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "or %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier xor_specs[] = {
    {0, "xor %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "xor %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "xor %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier lsh_specs[] = {
    {0, "lsh %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "lsh %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "lsh %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier rsh_specs[] = {
    {0, "rsh %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 3},
    {1, "rsh %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "rsh %rd, [%operand2]", "[sp(8)] [opcode(8)] [rd(8)] [operand2(32)]", 4},
};

static const InstructionSpecifier mov_specs[] = {
    {0, "mov %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [immediate(16)]", 3},
    {1, "mov %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 2},
    {2, "mov %rd.L, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 4},
    {3, "mov %rd.H, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 4},
    {4, "mov %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 4},
    {5, "mov %rd, %rn1, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [rn1(8)] [normAddressing(32)]", 5},
    {6, "mov [%normAddressing], %rd.L", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 4},
    {7, "mov [%normAddressing], %rd.H", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 4},
    {8, "mov [%normAddressing], %rd", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 4},
    {9, "mov [%normAddressing], %rd, %rn1", "[sp(8)] [opcode(8)] [rd(8)] [rn1(8)] [normAddressing(32)]", 5},
    {10, "mov %rd.L, [%rn + #offset]", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(16)]", 5},
    {11, "mov %rd.H, [%rn + #offset]", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(16)]", 5},
    {12, "mov %rd, [%rn + #offset]", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(16)]", 5},
    {13, "mov %rd, %rn1, [%rn + #offset]", "[sp(8)] [opcode(8)] [rd(8)] [rn1(8)] [rn(8)] [offset(16)]", 6},
    {14, "mov [%rn + #offset], %rd.L", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(16)]", 5},
    {15, "mov [%rn + #offset], %rd.H", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(16)]", 5},
    {16, "mov [%rn + #offset], %rd", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(16)]", 5},
    {17, "mov [%rn + #offset], %rd, %rn1", "[sp(8)] [opcode(8)] [rd(8)] [rn1(8)] [rn(8)] [offset(16)]", 6},
};

static const InstructionFormat instructions[] = {
    {"nop", 0x00, 1, nop_specs},
    {"add", 0x01, 3, add_specs},
    {"sub", 0x02, 3, sub_specs},
    {"mul", 0x03, 3, mul_specs},
    {"and", 0x04, 3, and_specs},
    {"or", 0x05, 3, or_specs},
    {"xor", 0x06, 3, xor_specs},
    {"lsh", 0x07, 3, lsh_specs},
    {"rsh", 0x08, 3, rsh_specs},
    {"mov", 0x00, 18, mov_specs},
};

#endif // INSTRUCTIONS_H
