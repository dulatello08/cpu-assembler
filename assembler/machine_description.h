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
    {0, "nop", "[sp(8)] [opcode(8)]", 2},
};

static const InstructionSpecifier add_specs[] = {
    {0, "add %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "add %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "add %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier sub_specs[] = {
    {0, "sub %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "sub %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "sub %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier mul_specs[] = {
    {0, "mul %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "mul %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "mul %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier and_specs[] = {
    {0, "and %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "and %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "and %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier or_specs[] = {
    {0, "or %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "or %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "or %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier xor_specs[] = {
    {0, "xor %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "xor %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "xor %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier lsh_specs[] = {
    {0, "lsh %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "lsh %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "lsh %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier rsh_specs[] = {
    {0, "rsh %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [operand2(16)]", 5},
    {1, "rsh %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "rsh %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
};

static const InstructionSpecifier mov_specs[] = {
    {0, "mov %rd, #%immediate", "[sp(8)] [opcode(8)] [rd(8)] [immediate(16)]", 5},
    {1, "mov %rd, %rn", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)]", 4},
    {2, "mov %rd.L, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
    {3, "mov %rd.H, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
    {4, "mov %rd, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
    {5, "mov %rd, %rn1, [%normAddressing]", "[sp(8)] [opcode(8)] [rd(8)] [rn1(8)] [normAddressing(32)]", 8},
    {6, "mov [%normAddressing], %rd.L", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
    {7, "mov [%normAddressing], %rd.H", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
    {8, "mov [%normAddressing], %rd", "[sp(8)] [opcode(8)] [rd(8)] [normAddressing(32)]", 7},
    {9, "mov [%normAddressing], %rd, %rn1", "[sp(8)] [opcode(8)] [rd(8)] [rn1(8)] [normAddressing(32)]", 8},
    {10, "mov %rd.L, [%rn + #%offset]", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(32)]", 8},
    {11, "mov %rd.H, [%rn + #%offset]", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(32)]", 8},
    {12, "mov %rd, [%rn + #%offset]", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(32)]", 8},
    {13, "mov %rd, %rd1, [%rn + #%offset]", "[sp(8)] [opcode(8)] [rd(8)] [rd1(8)] [rn(8)] [offset(32)]", 9},
    {14, "mov [%rn + #%offset], %rd.L", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(32)]", 8},
    {15, "mov [%rn + #%offset], %rd.H", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(32)]", 8},
    {16, "mov [%rn + #%offset], %rd", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [offset(32)]", 8},
    {17, "mov [%rn + #%offset], %rd, %rn1", "[sp(8)] [opcode(8)] [rd(8)] [rn1(8)] [rn(8)] [offset(32)]", 9},
};

static const InstructionSpecifier b_specs[] = {
    {0, "b %label", "[sp(8)] [opcode(8)] [label(32)]", 6},
};

static const InstructionSpecifier be_specs[] = {
    {0, "be %rd, %rn, %label", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [label(32)]", 8},
};

static const InstructionSpecifier bne_specs[] = {
    {0, "bne %rd, %rn, %label", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [label(32)]", 8},
};

static const InstructionSpecifier blt_specs[] = {
    {0, "blt %rd, %rn, %label", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [label(32)]", 8},
};

static const InstructionSpecifier bgt_specs[] = {
    {0, "bgt %rd, %rn, %label", "[sp(8)] [opcode(8)] [rd(8)] [rn(8)] [label(32)]", 8},
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
    {"mov", 0x09, 18, mov_specs},
    {"b", 0x0A, 1, b_specs},
    {"be", 0x0B, 1, be_specs},
    {"bne", 0x0C, 1, bne_specs},
    {"blt", 0x0D, 1, blt_specs},
    {"bgt", 0x0E, 1, bgt_specs},
};

#endif // INSTRUCTIONS_H
