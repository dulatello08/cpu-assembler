//
// Created by Dulat S on 2/17/24.
//

#ifndef LINKER_H
#define LINKER_H


#include <vector>
#include <cstdint>

struct LabelInfo {
    std::string name;
    uint32_t address;
};
struct RelocationInfo {
    uint16_t absolute_index;
    uint32_t address;
};

void print_hex_dump(const std::vector<uint8_t>& object_file);

#endif // LINKER_H