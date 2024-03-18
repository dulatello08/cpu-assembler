//
// Created by Dulat S on 3/3/24.
//

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H
#include <vector>
#include <cstdint>
#include <map>


class memory_layout {
    std::vector<std::vector<uint8_t>> object_files;
    std::vector<std::vector<uint8_t>> object_codes;
    std::map<std::string, std::tuple<int, int, int>> label_ranges;
    std::map<std::string, std::vector<uint8_t>> label_codes;
public:
    std::vector<uint8_t> memory;
    explicit memory_layout(const std::vector<std::vector<uint8_t>>& object_files) : object_files(object_files) {
        extract_object_codes();
        label_ranges = find_label_ranges();
        label_codes = extract_label_codes();
        write_memory_layout();
    }
private:
    void extract_object_codes();
    std::map<std::string, std::tuple<int, int, int>> find_label_ranges();
    void write_memory_layout();
    std::map<std::string, std::vector<uint8_t>> extract_label_codes();
};


#endif //MEMORY_LAYOUT_H
