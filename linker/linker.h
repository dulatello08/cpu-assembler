//
// Created by Dulat S on 2/17/24.
//

#ifndef LINKER_H
#define LINKER_H


#include <vector>

struct linker_config {
    uint16_t start_label_address;
    bool output_256_bytes_boot_sector;
    bool output_4_kb_flash;
};

struct LabelInfo {
    std::string name;
    uint16_t address;
};

void print_hex_dump(const std::vector<uint8_t>& object_file);

#endif // LINKER_H