#ifndef MISC_H
#define MISC_H

#include <iostream>

#include "types.h"

namespace harukashogi {


constexpr std::string_view PieceToChar(" KGSLNBRP      kgslnbrp");


constexpr int PieceValues[NUM_PIECE_TYPES] = {
    0, 6, 5, 4, 3, 10, 12, 1,
    6, 6, 6, 16, 16, 6
};


// operators to convert between types and strings
std::ostream& operator<<(std::ostream& os, Square t);
std::ostream& operator<<(std::ostream& os, Move m);


Move move_from_string(const std::string& move_str);


enum LogLevel : uint8_t {
    SILENT,
    ESSENTIAL,
    INFO,
    DEBUG,
};


} // namespace harukashogi

#endif // MISC_H