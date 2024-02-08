//
// Created by gitpod on 2/8/24.
//

#ifndef CPU_ASSEMBLER_CONF_PARSER_H
#define CPU_ASSEMBLER_CONF_PARSER_H


#include <vector>
#include <cstdint>
#include <string>

class conf_parser {
public:
    std::vector<uint8_t> confFile;
    uint8_t getOpCode(const std::string& instruction) {
        
        return 0x0;
    }
};


#endif //CPU_ASSEMBLER_CONF_PARSER_H
