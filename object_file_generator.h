#include <utility>

//
// Created by Dulat S on 2/13/24.
//

#ifndef OBJECT_FILE_GENERATOR_H
#define OBJECT_FILE_GENERATOR_H


class ObjectFileGenerator {
    Parser::Metadata metadata;
    std::vector<uint8_t> object_code;
    std::vector<Parser::RelocationEntry> relocation_table;

public:
    ObjectFileGenerator(Parser:: Metadata  metadata, const std::vector<uint8_t>& object_code, const std::vector<Parser::RelocationEntry>& relocation_table)
        : metadata(std::move(metadata)), object_code(object_code), relocation_table(relocation_table) {}

    void generate_object_file(const std::string& file_name) {
        std::ofstream file(file_name, std::ios::binary | std::ios::out);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << file_name << std::endl;
            return;
        }

        // Write metadata
        write_string(file, metadata.compiler_version);
        auto date_of_compilation = static_cast<unsigned long>(metadata.date_of_compilation);
        file.write(reinterpret_cast<const char*>(&date_of_compilation), sizeof(date_of_compilation));
        write_string(file, metadata.source_file_name);

        // Write object code
        auto object_code_size = static_cast<uint32_t>(object_code.size());
        file.write(reinterpret_cast<const char*>(&object_code_size), sizeof(object_code_size));
        file.write(reinterpret_cast<const char*>(object_code.data()), object_code_size);

        // Write relocation table
        auto relocation_table_size = static_cast<uint16_t>(relocation_table.size());
        file.write(reinterpret_cast<const char*>(&relocation_table_size), sizeof(relocation_table_size));
        for (const auto& entry : relocation_table) {
            write_string(file, entry.label);
            file.write(reinterpret_cast<const char*>(&entry.address), sizeof(entry.address));
        }

        file.close();
    }

private:
    static void write_string(std::ofstream& file, const std::string& str) {
        file.write(str.c_str(), str.size() + 1); // +1 to include null terminator
    }
};



#endif //OBJECT_FILE_GENERATOR_H
