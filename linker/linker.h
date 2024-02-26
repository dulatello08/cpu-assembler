//
// Created by Dulat S on 2/17/24.
//

#ifndef LINKER_H
#define LINKER_H


struct linker_config {
    uint16_t start_label_address;
    bool output_256_bytes_boot_sector;
    bool output_4_kb_flash;
};

#endif // LINKER_H