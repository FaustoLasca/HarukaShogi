#include <sstream>

#include "misc.h"
#include "bitboard.h"
#include "position.h"

namespace harukashogi {


void init() {
    Bitboards::init();
    Position::init();
}


std::ostream& operator<<(std::ostream& os, Square sq) {
    return os << int(file_of(sq) + 1) << char('a' + rank_of(sq));
}


std::string Move::to_string() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}


std::ostream& operator<<(std::ostream& os, Move m) {
    if (m.is_drop()) {
        os << PieceToChar[make_piece(BLACK, m.dropped())] << "*";
    }
    else {
        os << m.from();
    }

    os << m.to();

    if (m.is_promotion()) {
        os << "+";
    }

    return os;
}


} // namespace harukashogi