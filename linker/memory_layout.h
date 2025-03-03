//
// Created by Dulat S on 3/3/24.
//

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include <tuple>

#include "linker.h"  // Assuming this defines LabelInfo and RelocationInfo

class memory_layout {
    std::vector<std::vector<uint8_t>> object_files;
    std::map<std::string, std::tuple<int, int, int>> label_ranges;
    std::vector<std::vector<LabelInfo>> label_info_per_file;
    std::vector<std::vector<RelocationInfo>> relocation_info_per_file;
public:
    std::vector<uint8_t> memory;

    // Updated constructor that accepts both the object files and the parsed label/relocation info.
    memory_layout(const std::vector<std::vector<uint8_t>>& object_files,
                  const std::vector<std::vector<LabelInfo>>& label_info,
                  const std::vector<std::vector<RelocationInfo>>& relocation_info)
        : object_files(object_files),
          label_info_per_file(label_info),
          relocation_info_per_file(relocation_info)
    {
        extract_object_codes();
        relocate_memory_layout();
        print_hex_dump(memory);
    }
private:
    void extract_object_codes();
    void relocate_memory_layout();
};

#endif // MEMORY_LAYOUT_H