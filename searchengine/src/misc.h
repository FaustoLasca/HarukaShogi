#ifndef MISC_H
#define MISC_H

#include <iostream>
#include <random>

#include "types.h"

namespace harukashogi {


constexpr std::string_view PieceToChar(" KGBRSLNP      kgbrslnp");


// operators to convert between types and strings
std::ostream& operator<<(std::ostream& os, Square t);
std::ostream& operator<<(std::ostream& os, Move m);


void init();


} // namespace harukashogi

#endif // MISC_H