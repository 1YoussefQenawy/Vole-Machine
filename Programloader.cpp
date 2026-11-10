#include "VoleMachine.h"
 
#include <cctype>
#include <fstream>
#include <vector>
#include <sstream>
 
// File format: hex tokens separated by spaces/newlines.
//   4 hex digits (e.g. 1A3C) = one instruction (2 bytes)
//   2 hex digits (e.g. FF)   = one byte of data
// The file is fully validated BEFORE writing, so memory is never half-loaded.

bool ProgramLoader::load(std::string filename, int startAddress, Memory& memory) {
    if (!memory.isValidStart(startAddress)) return false;

    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::vector<Byte> bytes;
    std::string line;

    while (std::getline(file, line)) {
        // Remove comments (everything after the semicolon).
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Parse the remaining hexadecimal tokens.
        std::istringstream lineStream(line);
        std::string token;

        while (lineStream >> token) {
            if (token.size() != 2 && token.size() != 4) return false;

            for (char c : token) {
                if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
            }

            for (size_t i = 0; i < token.size(); i += 2) {
                bytes.push_back(
                    static_cast<Byte>(
                        std::stoi(token.substr(i, 2), nullptr, 16)
                    )
                );
            }
        }
    }

    // Validate memory boundaries before writing.
    if (startAddress + static_cast<int>(bytes.size()) > 256) return false;

    for (size_t i = 0; i < bytes.size(); i++) {
        memory.write(startAddress + static_cast<int>(i), bytes[i]);
    }

    return true;
}
