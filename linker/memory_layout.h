//
// Created by Dulat S on 3/3/24.
//

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H
#include <utility>
#include <vector>
#include <cstdint>
#include <map>

#include "linker.h"


class memory_layout {
    std::vector<std::vector<uint8_t>> object_files;
    std::vector<std::vector<uint8_t>> object_codes;
    std::map<std::string, std::tuple<int, int, int>> label_ranges;
    std::map<std::string, std::vector<uint8_t>> label_codes;
    std::map<std::string, uint16_t> label_memory_addresses; // Store memory addresses of labels
    std::vector<LabelInfo> label_info_per_file;
public:
    std::vector<uint8_t> memory;
    memory_layout(const std::vector<std::vector<uint8_t>>& object_files, std::vector<LabelInfo> label_info_per_file) : object_files(object_files), label_info_per_file(std::move(label_info_per_file)) {
        extract_object_codes();
        label_ranges = find_label_ranges();
        label_codes = extract_label_codes();
        write_memory_layout();
        relocate_memory_layout();
    }
private:
    void extract_object_codes();
    std::map<std::string, std::tuple<int, int, int>> find_label_ranges();
    void write_memory_layout();
    std::map<std::string, std::vector<uint8_t>> extract_label_codes();
    void relocate_memory_layout();
};


#endif //MEMORY_LAYOUT_H
