#include <sstream>

#include "misc.h"

namespace harukashogi {


std::ostream& operator<<(std::ostream& os, Square sq) {
    return os << int(file_of(sq) + 1) << char('a' + rank_of(sq));
}

std::ostream& operator<<(std::ostream& os, Move m) {
    if (m.is_drop()) {
        os << PieceToChar[make_piece(BLACK, m.type_involved)] << "*";
    }
    else {
        os << m.from;
    }

    os << m.to;

    if (m.promotion) {
        os << "+";
    }

    return os;
}


} // namespace harukashogi