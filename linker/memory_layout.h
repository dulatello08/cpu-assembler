//
// Created by Dulat S on 3/3/24.
//

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H
#include <vector>


class memory_layout {
    std::vector<std::vector<uint8_t>> object_files;
    std::vector<std::vector<uint8_t>> object_codes;
    std::pair<int, int> global_start_range;
public:
    std::vector<uint8_t> memory;
    explicit memory_layout(const std::vector<std::vector<uint8_t>>& object_files) : object_files(object_files) {
        extract_object_codes();
        global_start_range = find_global_start_range();
    }
private:
    void extract_object_codes();
    std::pair<int, int> find_global_start_range() const;
};


#endif //MEMORY_LAYOUT_H
