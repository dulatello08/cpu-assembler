//
// Created by Dulat S on 2/1/24.
//

#ifndef MAIN_H
#define MAIN_H

inline time_t get_compile_unix_time() {
    const char *compile_date = __DATE__; // "Mmm dd yyyy"
    const char *compile_time = __TIME__; // "hh:mm:ss"

    struct tm tm{};
    strptime(compile_date, "%b %d %Y", &tm);
    sscanf(compile_time, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

    // Set the fields not set by strptime.
    tm.tm_isdst = -1;  // Not dealing with daylight saving time

    return mktime(&tm); // Convert to Unix time
}

// Retrieve opcode for a given instruction name.
uint8_t get_opcode_for_instruction(const char* inst_name);

// Retrieve syntax based on instruction name and specifier 'sp'.
const char* get_syntax_for_instruction(const char* inst_name, uint8_t sp);

// Helper to find the InstructionFormat for a given instruction name.
const InstructionFormat* find_instruction_format(const char* inst_name)

// Retrieve encoding based on instruction name and specifier 'sp'.
const char* get_encoding_for_instruction(const char* inst_name, uint8_t sp);

// Retrieve length based on instruction name and specifier 'sp'.
uint8_t get_length_for_instruction(const char* inst_name, uint8_t sp);
#endif //MAIN_H
