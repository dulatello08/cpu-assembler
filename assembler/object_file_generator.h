#ifndef OBJECT_FILE_GENERATOR_H
#define OBJECT_FILE_GENERATOR_H

#include <algorithm> // For std::reverse
#include "code_generator.h"
#include <chrono>

//
// Created by Dulat S on 2/13/24.
//


/*
ObjectFileGenerator builds an object file in our custom “LF” format (big-endian) without a metadata block.
The file layout is as follows:

    +-----------------------------+
    | 1. Header (32 bytes)        |
    +-----------------------------+
    | 2. Machine Code Blob        |
    +-----------------------------+
    | 3. Label Table Block        |
    +-----------------------------+
    | 4. Relocation Table Block   |
    +-----------------------------+

The header layout (32 bytes):
  - Bytes 0-3:   Magic ("LF01")
  - Bytes 4-5:   Version (0x0001)
  - Bytes 6-7:   Flags (0x0000)
  - Bytes 8-15:  Timestamp (current time in microseconds)
  - Bytes 16-19: Machine Code Length
  - Bytes 20-23: Label Table Offset
  - Bytes 24-27: Relocation Table Offset
  - Bytes 28-31: Metadata Offset (set to 0, as no metadata block is generated)

Label strings are stored as a length field (which includes the terminating zero) followed by the
UTF‑8 characters and a trailing 0x00 byte.
*/
class ObjectFileGenerator {
public:
    // Constructor accepts the relocation table, label-to-address mapping, and machine code.
    ObjectFileGenerator(const std::vector<CodeGenerator::RelocationEntry>& relocationEntries,
                        const std::unordered_map<std::string, uint32_t>& labelTable,
                        const std::vector<uint8_t>& machineCode)
        : relocationEntries_(relocationEntries),
          labelTable_(labelTable),
          machineCode_(machineCode)
    {
    }

    // Builds and returns the complete object file as a vector of bytes.
    [[nodiscard]] std::vector<uint8_t> build() const {
        std::vector<uint8_t> buffer;
        // Reserve space for the fixed header (32 bytes).
        buffer.resize(32, 0);

        // --- Append the Machine Code Blob ---
        buffer.insert(buffer.end(), machineCode_.begin(), machineCode_.end());
        auto machineCodeLength = static_cast<uint32_t>(machineCode_.size());

        // --- Build the Label Table Block ---
        std::vector<uint8_t> labelTableBlock = buildLabelTableBlock();
        auto labelTableOffset = static_cast<uint32_t>(buffer.size());
        buffer.insert(buffer.end(), labelTableBlock.begin(), labelTableBlock.end());

        // --- Build the Relocation Table Block ---
        std::vector<uint8_t> relocationTableBlock = buildRelocationTableBlock();
        auto relocationTableOffset = static_cast<uint32_t>(buffer.size());
        buffer.insert(buffer.end(), relocationTableBlock.begin(), relocationTableBlock.end());

        // --- Now fill in the header fields ---
        // Header layout (32 bytes):
        //  0-3:   Magic ("LF01")
        //  4-5:   Version (0x0001)
        //  6-7:   Flags (0x0000)
        //  8-15:  Timestamp (current time in microseconds)
        // 16-19:  Machine Code Length
        // 20-23:  Label Table Offset
        // 24-27:  Relocation Table Offset
        // 28-31:  Metadata Offset (0, since metadata is not included)

        // Magic "LF01"
        writeBytes(buffer, 0, { 'L', 'F', '0', '1' });
        // Version: 0x0001
        writeUint16(buffer, 4, 0x0001);
        // Flags: 0x0000
        writeUint16(buffer, 6, 0x0000);
        // Timestamp: use system_clock now in microseconds.
        uint64_t timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        writeUint64(buffer, 8, timestamp);
        // Machine Code Length.
        writeUint32(buffer, 16, machineCodeLength);
        // Label Table Offset.
        writeUint32(buffer, 20, labelTableOffset);
        // Relocation Table Offset.
        writeUint32(buffer, 24, relocationTableOffset);
        // Metadata Offset (set to 0, since metadata is not included).
        writeUint32(buffer, 28, 0);

        return buffer;
    }

private:
    const std::vector<CodeGenerator::RelocationEntry>& relocationEntries_;
    const std::unordered_map<std::string, uint32_t>& labelTable_;
    const std::vector<uint8_t>& machineCode_;

    // --- Helper Functions for Writing Data in Big-Endian Format ---

    // Write a list of bytes at the given offset (assumes offset is valid).
    static void writeBytes(std::vector<uint8_t>& buffer, size_t offset, const std::initializer_list<uint8_t>& bytes) {
        size_t i = 0;
        for (auto b : bytes) {
            if (offset + i < buffer.size())
                buffer[offset + i] = b;
            else
                buffer.push_back(b);
            ++i;
        }
    }

    // Write a 16-bit unsigned integer (big-endian) into buffer at offset.
    static void writeUint16(std::vector<uint8_t>& buffer, size_t offset, uint16_t value) {
        if (offset + 1 >= buffer.size())
            throw std::out_of_range("Offset out of range in writeUint16");
        buffer[offset]   = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[offset+1] = static_cast<uint8_t>(value & 0xFF);
    }

    // Write a 32-bit unsigned integer (big-endian) into buffer at offset.
    static void writeUint32(std::vector<uint8_t>& buffer, size_t offset, uint32_t value) {
        if (offset + 3 >= buffer.size())
            throw std::out_of_range("Offset out of range in writeUint32");
        buffer[offset]   = static_cast<uint8_t>((value >> 24) & 0xFF);
        buffer[offset+1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[offset+2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[offset+3] = static_cast<uint8_t>(value & 0xFF);
    }

    // Write a 64-bit unsigned integer (big-endian) into buffer at offset.
    static void writeUint64(std::vector<uint8_t>& buffer, size_t offset, uint64_t value) {
        if (offset + 7 >= buffer.size())
            throw std::out_of_range("Offset out of range in writeUint64");
        buffer[offset]   = static_cast<uint8_t>((value >> 56) & 0xFF);
        buffer[offset+1] = static_cast<uint8_t>((value >> 48) & 0xFF);
        buffer[offset+2] = static_cast<uint8_t>((value >> 40) & 0xFF);
        buffer[offset+3] = static_cast<uint8_t>((value >> 32) & 0xFF);
        buffer[offset+4] = static_cast<uint8_t>((value >> 24) & 0xFF);
        buffer[offset+5] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[offset+6] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[offset+7] = static_cast<uint8_t>(value & 0xFF);
    }

    // --- Build the Label Table Block ---
    // Label table block layout:
    //   [Label Count (4 bytes)]
    //   For each label:
    //     [Code Offset (4 bytes)] [Name Length (2 bytes)] [Name (bytes including trailing 0x00)]
    // In this version, strings are stored as zero-terminated.
    [[nodiscard]] std::vector<uint8_t> buildLabelTableBlock() const {
        std::vector<uint8_t> block;
        // Convert unordered_map to a vector of pairs sorted by label name for stable ordering.
        std::vector<std::pair<std::string, uint32_t>> labels(labelTable_.begin(), labelTable_.end());
        std::ranges::sort(labels, [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        // Write label count.
        auto labelCount = static_cast<uint32_t>(labels.size());
        block.push_back(static_cast<uint8_t>((labelCount >> 24) & 0xFF));
        block.push_back(static_cast<uint8_t>((labelCount >> 16) & 0xFF));
        block.push_back(static_cast<uint8_t>((labelCount >> 8) & 0xFF));
        block.push_back(static_cast<uint8_t>(labelCount & 0xFF));

        // Write each label entry.
        for (const auto& entry : labels) {
            // Code offset (4 bytes).
            uint32_t offset = entry.second;
            block.push_back(static_cast<uint8_t>((offset >> 24) & 0xFF));
            block.push_back(static_cast<uint8_t>((offset >> 16) & 0xFF));
            block.push_back(static_cast<uint8_t>((offset >> 8) & 0xFF));
            block.push_back(static_cast<uint8_t>(offset & 0xFF));

            // Insert the string characters.
            block.insert(block.end(), entry.first.begin(), entry.first.end());
            // Append the zero terminator.
            block.push_back(0x00);
        }
        return block;
    }

    // --- Build the Relocation Table Block ---
    // Relocation table block layout:
    //   [Relocation Entry Count (4 bytes)]
    //   For each relocation:
    //     [Code Offset (4 bytes)] [Reloc Type (1 byte)] [Reserved (1 byte)] [Label Index (2 bytes)]
    // In this example, we use a default relocation type of 0.
    // The label index is determined by finding the relocation entry's label in the sorted label table.
    [[nodiscard]] std::vector<uint8_t> buildRelocationTableBlock() const {
        std::vector<uint8_t> block;
        // First, obtain the sorted labels as in the label table.
        std::vector<std::pair<std::string, uint32_t>> labels(labelTable_.begin(), labelTable_.end());
        std::ranges::sort(labels, [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        // Write relocation entry count.
        auto relocCount = static_cast<uint32_t>(relocationEntries_.size());
        block.push_back(static_cast<uint8_t>((relocCount >> 24) & 0xFF));
        block.push_back(static_cast<uint8_t>((relocCount >> 16) & 0xFF));
        block.push_back(static_cast<uint8_t>((relocCount >> 8) & 0xFF));
        block.push_back(static_cast<uint8_t>(relocCount & 0xFF));

        // Write each relocation entry.
        for (const auto& reloc : relocationEntries_) {
            // Code offset (4 bytes).
            uint32_t codeOffset = reloc.address;
            block.push_back(static_cast<uint8_t>((codeOffset >> 24) & 0xFF));
            block.push_back(static_cast<uint8_t>((codeOffset >> 16) & 0xFF));
            block.push_back(static_cast<uint8_t>((codeOffset >> 8) & 0xFF));
            block.push_back(static_cast<uint8_t>(codeOffset & 0xFF));

            // Determine label index from sortedLabels.
            auto it = std::ranges::find_if(labels,
                                           [&reloc](const std::pair<std::string, uint32_t>& pair) {
                                               return pair.first == reloc.label;
                                           });
            if (it == labels.end()) {
                throw std::runtime_error("Relocation entry refers to unknown label: " + reloc.label);
            }
            auto labelIndex = static_cast<uint16_t>(std::distance(labels.begin(), it));
            block.push_back(static_cast<uint8_t>((labelIndex >> 8) & 0xFF));
            block.push_back(static_cast<uint8_t>(labelIndex & 0xFF));
        }
        return block;
    }
};

#endif //OBJECT_FILE_GENERATOR_H
