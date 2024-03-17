//
// Created by Dulat S on 3/3/24.
//

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H
#include <vector>
#include <cstdint>
#include "linker.h"


class memory_layout {
    std::vector<std::vector<uint8_t>> object_files;
    std::vector<std::vector<uint8_t>> object_codes;
    std::map<std::string, std::pair<int, int>> label_ranges;
public:
    std::vector<uint8_t> memory;
    explicit memory_layout(const std::vector<std::vector<uint8_t>>& object_files) : object_files(object_files) {
        extract_object_codes();
        label_ranges = find_label_ranges();
        write_memory_layout();
    }
private:
    void extract_object_codes();
    std::map<std::string, std::pair<int, int>> find_label_ranges() const;
    void write_memory_layout();
};


#endif //MEMORY_LAYOUT_H
