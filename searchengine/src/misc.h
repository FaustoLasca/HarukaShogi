#ifndef MISC_H
#define MISC_H

#include <iostream>

#include "types.h"

namespace harukashogi {


constexpr std::string_view PieceToChar(" KGSLNBRP      kgslnbrp");


// operators to convert between types and strings
std::ostream& operator<<(std::ostream& os, Square t);
std::ostream& operator<<(std::ostream& os, Move pt);


void init();


} // namespace harukashogi

#endif // MISC_H