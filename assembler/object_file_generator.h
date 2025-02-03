#include <algorithm> // For std::reverse
#include <utility>
#include <bit>
#include "parser.h"

//
// Created by Dulat S on 2/13/24.
//

#ifndef OBJECT_FILE_GENERATOR_H
#define OBJECT_FILE_GENERATOR_H


class ObjectFileGenerator {
    Parser::Metadata metadata;
    std::vector<uint8_t> object_code;
    std::vector<Parser::RelocationEntry> relocation_table;
    std::vector<uint8_t> create_object_file_buffer() {
        std::vector<uint8_t> buffer;

        // Helper lambda to append data to the buffer
        auto append_to_buffer = [&buffer](const void* data, size_t size) {
            const auto* bytes = static_cast<const uint8_t*>(data);
            buffer.insert(buffer.end(), bytes, bytes + size);
        };

        // Write metadata
        append_to_buffer(metadata.compiler_version.c_str(), metadata.compiler_version.size() + 1);
        auto date_of_compilation = static_cast<unsigned long>(metadata.date_of_compilation);
        auto* date_ptr = reinterpret_cast<unsigned char*>(&date_of_compilation);

        // Convert to big-endian if the system is little-endian
        if constexpr (std::endian::native == std::endian::little) {
            std::reverse(date_ptr, date_ptr + sizeof(date_of_compilation));
        }

        append_to_buffer(date_ptr, sizeof(date_of_compilation));
        append_to_buffer(metadata.source_file_name.c_str(), metadata.source_file_name.size() + 1);

        // Write object code
        auto object_code_size = static_cast<uint32_t>(object_code.size());
        append_to_buffer(&object_code_size, sizeof(object_code_size));
        append_to_buffer(object_code.data(), object_code.size());

        // Write relocation table
        auto relocation_table_size = static_cast<uint16_t>(relocation_table.size());
        append_to_buffer(&relocation_table_size, sizeof(relocation_table_size));
        for (const auto& entry : relocation_table) {
            append_to_buffer(entry.label.c_str(), entry.label.size() + 1);
            append_to_buffer(&entry.address, sizeof(entry.address));
        }

        return buffer;
    }

public:
    ObjectFileGenerator(Parser:: Metadata  metadata, const std::vector<uint8_t>& object_code, const std::vector<Parser::RelocationEntry>& relocation_table)
        : metadata(std::move(metadata)), object_code(object_code), relocation_table(relocation_table) {}

    void generate_object_file(const std::string& file_name) {
        std::ofstream file(file_name, std::ios::binary | std::ios::out);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << file_name << std::endl;
            return;
        }

        std::vector<uint8_t> buffer = create_object_file_buffer();
        file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<long>(buffer.size()));
        file.close();
    }

    std::vector<uint8_t> get_object_file() {
        return create_object_file_buffer();
    }
};



#endif //OBJECT_FILE_GENERATOR_H
