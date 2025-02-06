//
// Created by Dulat S on 2/18/24.
//

#include "object_files_parser.h"
#include <arpa/inet.h>
#include <ctime>
#include <sstream>
#include <iostream>
#include <map>
#include <cstring>
#include <algorithm>

#if defined(__linux__) && !defined(__APPLE__)
#include <cstdint>
#include <endian.h>

inline uint64_t htonll(uint64_t x) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return __builtin_bswap64(x);
#else
    return x;
#endif
}
#endif

bool object_files_parser::validate_all_files() {
    // Clear any existing data.
    label_info_per_file.clear();
    relocation_info_per_file.clear();

    // Process each object file.
    for (size_t i = 0; i < object_file_vectors.size(); ++i) {
        // Create an input stream from the file vector data.
        std::istringstream file_stream(
            std::string(object_file_vectors[i].begin(), object_file_vectors[i].end()),
            std::ios::binary
        );

        // --- Read the 32-byte header ---
        // Bytes 0-3: Magic ("LF01")
        char magic[5] = {0};
        file_stream.read(magic, 4);
        if (!file_stream || std::string(magic, 4) != "LF01") {
            log_error("Invalid or missing magic header", i);
            return false;
        }

        // Bytes 4-5: Version (0x0001)
        std::uint16_t version;
        file_stream.read(reinterpret_cast<char *>(&version), sizeof(version));
        version = ntohs(version);
        if (!file_stream || version != 0x0001) {
            log_error("Unsupported or missing version", i);
            return false;
        }

        // Bytes 6-7: Flags.
        std::uint16_t flags;
        file_stream.read(reinterpret_cast<char *>(&flags), sizeof(flags));
        flags = ntohs(flags);
        if (!file_stream) {
            log_error("Could not read flags", i);
            return false;
        }

        // Bytes 8-15: Timestamp.
        std::uint64_t timestamp;
        file_stream.read(reinterpret_cast<char *>(&timestamp), sizeof(timestamp));
        timestamp = htonll(timestamp); // Assuming htonll is defined.
        if (!file_stream) {
            log_error("Could not read timestamp", i);
            return false;
        }
        time_t time_sec = static_cast<time_t>(timestamp) / 1000000;
        char time_buffer[80];
        std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %I:%M:%S %p", std::localtime(&time_sec));
        log_info("Timestamp: " + std::string(time_buffer), i);

        // Bytes 16-19: Machine Code Length.
        std::uint32_t machine_code_length;
        file_stream.read(reinterpret_cast<char *>(&machine_code_length), sizeof(machine_code_length));
        machine_code_length = ntohl(machine_code_length);
        if (!file_stream) {
            log_error("Could not read machine code length", i);
            return false;
        }
        log_info("Machine code length: " + std::to_string(machine_code_length), i);

        // Bytes 20-23: Label Table Offset.
        std::uint32_t label_table_offset;
        file_stream.read(reinterpret_cast<char *>(&label_table_offset), sizeof(label_table_offset));
        label_table_offset = ntohl(label_table_offset);
        if (!file_stream) {
            log_error("Could not read label table offset", i);
            return false;
        }
        log_info("Label table offset: " + std::to_string(label_table_offset), i);

        // Bytes 24-27: Relocation Table Offset.
        std::uint32_t relocation_table_offset;
        file_stream.read(reinterpret_cast<char *>(&relocation_table_offset), sizeof(relocation_table_offset));
        relocation_table_offset = ntohl(relocation_table_offset);
        if (!file_stream) {
            log_error("Could not read relocation table offset", i);
            return false;
        }
        log_info("Relocation table offset: " + std::to_string(relocation_table_offset), i);

        // Bytes 28-31: Metadata Offset.
        std::uint32_t metadata_offset;
        file_stream.read(reinterpret_cast<char *>(&metadata_offset), sizeof(metadata_offset));
        metadata_offset = ntohl(metadata_offset);
        if (!file_stream) {
            log_error("Could not read metadata offset", i);
            return false;
        }
        log_info("Metadata offset: " + std::to_string(metadata_offset), i);

        // --- Validate the Machine Code Section ---
        file_stream.seekg(32 + machine_code_length, std::ios::beg);
        if (!file_stream) {
            log_error("Machine code section is incomplete", i);
            return false;
        }

        // --- Read the Label Table ---
        file_stream.seekg(label_table_offset, std::ios::beg);
        if (!file_stream) {
            log_error("Could not seek to label table", i);
            return false;
        }

        // Assume the label table starts with a 32-bit count.
        std::uint32_t label_count;
        file_stream.read(reinterpret_cast<char *>(&label_count), sizeof(label_count));
        label_count = ntohl(label_count);
        if (!file_stream) {
            log_error("Could not read label count", i);
            return false;
        }
        log_info("Label count: " + std::to_string(label_count), i);

        std::vector<LabelInfo> labels_in_file;
        for (std::uint32_t j = 0; j < label_count; ++j) {
            LabelInfo label_info;
            // Read the 4-byte label address.
            file_stream.read(reinterpret_cast<char *>(&label_info.address), sizeof(label_info.address));
            label_info.address = ntohl(label_info.address);
            if (!file_stream) {
                log_error("Could not read label address", i);
                return false;
            }
            // Read a null-terminated label name.
            std::getline(file_stream, label_info.name, '\0');
            if (label_info.name.empty()) {
                log_error("Missing label name in label table", i);
                return false;
            }
            log_info("Label: " + label_info.name + ", Address: " + std::to_string(label_info.address), i);
            labels_in_file.push_back(std::move(label_info));
        }
        label_info_per_file.push_back(std::move(labels_in_file));

        // --- Read the Relocation Table ---
        file_stream.seekg(relocation_table_offset, std::ios::beg);
        if (!file_stream) {
            log_error("Could not seek to relocation table", i);
            return false;
        }

        std::uint32_t relocation_count;
        file_stream.read(reinterpret_cast<char *>(&relocation_count), sizeof(relocation_count));
        relocation_count = ntohl(relocation_count);
        log_info("Relocation count: " + std::to_string(relocation_count), i);

        std::vector<RelocationInfo> relocations_in_file;
        for (std::uint32_t j = 0; j < relocation_count; ++j) {
            RelocationInfo reloc_info{};
            // Read the 4-byte relocation address.
            file_stream.read(reinterpret_cast<char *>(&reloc_info.address), sizeof(reloc_info.address));
            reloc_info.address = ntohl(reloc_info.address);
            if (!file_stream) {
                log_error("Could not read relocation address", i);
                return false;
            }

            // Save the current position.
            std::streampos pos = file_stream.tellg();
            // Try reading a candidate string up to the null terminator.
            std::string candidate;
            std::getline(file_stream, candidate, '\0');

            // Check if the candidate is a plausible external label.
            bool candidateIsExternal = false;
            if (!candidate.empty() &&
                candidate.size() >= 3 &&
                std::all_of(candidate.begin(), candidate.end(), [](char c) {
                    return std::isalnum(c) || c == '_';
                }))
            {
                candidateIsExternal = true;
            }

            if (candidateIsExternal) {
                // External relocation: store the label string.
                reloc_info.is_external = true;
                reloc_info.external_label = candidate;
                // The label_location will be resolved later.
            } else {
                // Not external: roll back and read exactly 2 bytes as the file-local index.
                file_stream.clear(); // Clear any eof/fail flags.
                file_stream.seekg(pos);
                file_stream.read(reinterpret_cast<char *>(&reloc_info.local_index),
                                 sizeof(reloc_info.local_index));
                reloc_info.local_index = ntohs(reloc_info.local_index);
                reloc_info.is_external = false;
                // The label_location will be set later.
            }

            if (reloc_info.is_external) {
                log_info("Relocation: Address = " + std::to_string(reloc_info.address) +
                         ", External Label = " + reloc_info.external_label, i);
            } else {
                log_info("Relocation: Address = " + std::to_string(reloc_info.address) +
                         ", File-local Label Index = " + std::to_string(reloc_info.local_index), i);
            }
            relocations_in_file.push_back(std::move(reloc_info));
        }
        relocation_info_per_file.push_back(std::move(relocations_in_file));
    } // End processing all files

    // --- Post-process relocations ---
    // For each relocation, assign its location in the 2D label table.
    for (size_t file_index = 0; file_index < relocation_info_per_file.size(); ++file_index) {
        for (auto &reloc : relocation_info_per_file[file_index]) {
            if (reloc.is_external) {
                // For external relocations, search all files’ label tables.
                bool found = false;
                for (size_t otherFileIndex = 0; otherFileIndex < label_info_per_file.size(); ++otherFileIndex) {
                    const auto &labels = label_info_per_file[otherFileIndex];
                    for (std::uint16_t j = 0; j < labels.size(); ++j) {
                        if (labels[j].name == reloc.external_label) {
                            reloc.label_location = {otherFileIndex, j};
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found) {
                    log_error("Could not match external label: " + reloc.external_label, file_index);
                    // Depending on your error policy you may wish to return false here.
                }
                // Optionally clear the temporary external label.
                reloc.external_label.clear();
            } else {
                // For internal relocations, the label is in the same file.
                // Simply record the current file index and the local label index.
                // (A bounds check here might be a good idea.)
                if (reloc.local_index >= label_info_per_file[file_index].size()) {
                    log_error("Internal relocation index out of bounds", file_index);
                    return false;
                }
                reloc.label_location = {file_index, reloc.local_index};
            }
        }
    }

    std::cout << "All files validated successfully." << std::endl;
    return true;
}

void object_files_parser::log_label_info() const {
    // Log each file's labels and then its relocation entries.
    for (size_t file_index = 0; file_index < object_files.size(); ++file_index) {
        std::cout << "File: " << object_files[file_index] << "\n";

        // Print labels for this file.
        if (file_index < label_info_per_file.size()) {
            const auto &labels = label_info_per_file[file_index];
            for (size_t local_index = 0; local_index < labels.size(); ++local_index) {
                std::cout << "  Label " << local_index
                          << " [Address: " << labels[local_index].address << "] - "
                          << labels[local_index].name << "\n";
            }
        }

        // Now, log relocation info for this file.
        // (Assuming relocation_info_per_file is indexed in the same order as object_files.)
        if (file_index < relocation_info_per_file.size()) {
            const auto &relocs = relocation_info_per_file[file_index];
            for (const auto &reloc : relocs) {
                std::string label_name = "<unknown>";
                size_t target_file = reloc.label_location.first;
                size_t local_index = reloc.label_location.second;
                if (target_file < label_info_per_file.size() &&
                    local_index < label_info_per_file[target_file].size()) {
                    label_name = label_info_per_file[target_file][local_index].name;
                    }
                std::cout << "  Relocation: Address = " << reloc.address
                          << ", Label Location = (" << target_file << ", " << local_index << ")"
                          << " (" << label_name << ")\n";
            }
        }
    }
}