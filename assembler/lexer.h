// //
// // Created by Dulat S on 2/1/24.
// //
//
// #ifndef LEXER_H
// #define LEXER_H
//
// #include <map>
// #include <regex>
// #include <string>
// #include <utility>
// #include "assembler.h"
//
// class Lexer {
//     std::regex labelPattern;
//     std::regex macroPattern;
//     std::regex instructionPattern;
//     std::regex operandPattern;
//     std::regex commentPattern;
//     std::vector<uint8_t> conf;
//
// public:
//     Lexer()
//             : labelPattern(R"(^\.(\S+))"),
//               macroPattern(R"(^([A-Za-z_]\w*)\s*=\s*\$([0-9A-Xa-x]+))"),
//               instructionPattern(R"(^\s*([A-Z]{3,}))"), // Matches instruction mnemonics.
//               operandPattern(R"(\s+([^;,\s]+))"), // Matches operands and registers.
//               commentPattern(";.*$") // Matches comments for removal.
//     {};
//     explicit Lexer(std::vector<uint8_t> conf)
//             : labelPattern(R"(^\.(\S+))"),
//               macroPattern(R"(^([A-Za-z_]\w*)\s*=\s*\$([0-9A-Xa-x]+))"),
//               instructionPattern(R"(^\s*([A-Z]{3,}))"), // Matches instruction mnemonics.
//               operandPattern(R"(\s+([^;,\s]+))"), // Matches operands and registers.
//               commentPattern(";.*$"), // Matches comments for removal.
//               conf(std::move(conf))
//     {};
//     std::map<std::string, int> labelTable;
//     std::map<std::string, int> macroTable;
//     std::map<uint16_t, uint16_t> lineNumberToAddressMap;
//     std::vector<Tok`en> tokens;
//
//     void firstPass(std::vector<std::string> &lines);
//
//     void lex(const std::vector<std::string>& lines);
//     void classifyAndCreateToken(const std::string& operand);
// };
//
// #endif //LEXER_H