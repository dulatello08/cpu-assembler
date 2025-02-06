//
// Created by Dulat S on 3/3/24.
//

#include "memory_layout.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>

#include "linker.h"
#include <cstdint>
#include <vector>
#include <arpa/inet.h>  // For ntohl

void memory_layout::extract_object_codes() {
    // For each object file...
    for (const auto &file: object_files) {
        // Create a binary input stream from the raw file data.
        std::istringstream file_stream(
            std::string(file.begin(), file.end()),
            std::ios::binary
        );

        // The header layout is as follows:
        // Bytes  0-3: Magic ("LF01")
        // Bytes  4-5: Version (0x0001)
        // Bytes  6-7: Flags.
        // Bytes  8-15: Timestamp.
        // Bytes 16-19: Machine Code Length.
        // Bytes 20-23: Label Table Offset.
        // Bytes 24-27: Relocation Table Offset.
        // Bytes 28-31: Metadata Offset.
        //
        // We only need the machine code length, which is stored at offset 16.

        // Seek to byte offset 16.
        file_stream.seekg(16, std::ios::beg);
        if (!file_stream) {
            // Handle error: could not seek to machine code length field.
            continue;
        }

        // Read the 4-byte machine code length.
        std::uint32_t machine_code_length;
        file_stream.read(reinterpret_cast<char *>(&machine_code_length), sizeof(machine_code_length));
        if (!file_stream) {
            // Handle error: could not read machine code length.
            continue;
        }
        machine_code_length = ntohl(machine_code_length);

        // The machine code itself starts at offset 32.
        file_stream.seekg(32, std::ios::beg);
        if (!file_stream) {
            // Handle error: could not seek to machine code section.
            continue;
        }

        // Read the machine code bytes into a vector.
        std::vector<uint8_t> object_code(machine_code_length);
        file_stream.read(reinterpret_cast<char *>(object_code.data()), machine_code_length);
        if (!file_stream) {
            // Handle error: incomplete machine code read.
            continue;
        }

        // Store the object code.
        object_codes.push_back(std::move(object_code));
    }
}

std::map<std::string, std::tuple<int, int, int>> memory_layout::find_label_ranges() const {
    std::map<std::string, std::tuple<int, int, int>> label_ranges;

    // Process each file.
    for (size_t file_index = 0; file_index < object_files.size(); ++file_index) {
        // Retrieve the machine code length for this file.
        // object_codes[file_index] is assumed to hold only the machine code bytes.
        int code_length = static_cast<int>(object_codes[file_index].size());

        // Get the label list for the current file.
        // We copy it locally so we can sort it without modifying the original order.
        std::vector<LabelInfo> labels = label_info_per_file[file_index];

        // Sort labels by their address (in ascending order).
        std::ranges::sort(labels, [](const LabelInfo &a, const LabelInfo &b) {
            return a.address < b.address;
        });

        // For each label in the sorted list, determine its code range.
        for (size_t i = 0; i < labels.size(); ++i) {
            int start = static_cast<int>(labels[i].address);
            int end = code_length; // Default for the last label.
            if (i + 1 < labels.size()) {
                // The next label’s address marks the end of this label's range.
                end = static_cast<int>(labels[i + 1].address);
            }
            // Insert into the map.
            // The tuple stores: (file index, start, end).
            label_ranges[labels[i].name] = std::make_tuple(static_cast<int>(file_index), start, end);
        }
    }
    return label_ranges;
}

std::map<std::string, std::vector<uint8_t> > memory_layout::extract_label_codes() {
    std::map<std::string, std::vector<uint8_t>> label_codes;

    for (const auto &[label_name, range]: label_ranges) {
        auto [file_index, start, end] = range;

        std::vector<uint8_t> code;
        if (start <= end) {
            code = std::vector<uint8_t>(object_codes[file_index].begin() + start,
                                        object_codes[file_index].begin() + end);
        } else {
            // Extract in reverse order if start is greater than end
            code = std::vector<uint8_t>(object_codes[file_index].rbegin() + (object_codes[file_index].size() - start),
                                        object_codes[file_index].rbegin() + (object_codes[file_index].size() - end));
        }

        label_codes[label_name] = std::move(code);
    }

    return label_codes;
}


void memory_layout::write_memory_layout() {

}

void memory_layout::relocate_memory_layout() {
    // for (size_t i = 0; i < memory.size(); ++i) {
    //     if (memory[i] == 0xea) {
    //         if (i + 1 < memory.size()) {
    //             uint8_t label_index = memory[i + 1];
    //             if (label_index < label_info_per_file.size()) {
    //                 const auto &label_info = label_info_per_file[label_index];
    //
    //                 auto it = label_memory_addresses.find(label_info.name);
    //                 if (it != label_memory_addresses.end()) {
    //                     uint16_t label_address = it->second;
    //                     // Convert to big endian
    //                     memory[i] = static_cast<uint8_t>((label_address >> 8) & 0xFF);
    //                     memory[i + 1] = static_cast<uint8_t>(label_address & 0xFF);
    //                 }
    //             }
    //         }
    //         // Skip the next byte as it has already been processed
    //         ++i;
    //     }
    // }
}
