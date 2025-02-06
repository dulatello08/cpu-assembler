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
    std::uint32_t address;
    bool is_external;          // true if the relocation refers to an external label.
    std::uint16_t local_index; // valid if is_external == false.
    std::string external_label; // valid if is_external == true.
    // Instead of a global absolute index, we now store a pair:
    // first: file index (i.e., which file’s label table),
    // second: label index within that file.
    std::pair<size_t, std::uint16_t> label_location;
};


void print_hex_dump(const std::vector<uint8_t>& object_file);

#endif // LINKER_H