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

    if (!m.is_drop())
        if (m.is_promotion())
            os << "+";

    return os;
}


Move move_from_string(const std::string& moveStr) {
    if (moveStr.size() < 4) return Move::null();

    // Drop move: e.g. "P*5e"
    if (moveStr[1] == '*') {
        size_t pos = PieceToChar.find(moveStr[0]);
        if (pos == std::string_view::npos) return Move::null();
        PieceType pt = PieceType(pos - 1);
        Square to = make_square(File(moveStr[2] - '1'), Rank(moveStr[3] - 'a'));
        return Move(pt, to);
    }

    // Normal move: e.g. "7g7f" or "7g7f+"
    else {
        Square from = make_square(File(moveStr[0] - '1'), Rank(moveStr[1] - 'a'));
        Square to   = make_square(File(moveStr[2] - '1'), Rank(moveStr[3] - 'a'));
        bool promotion = moveStr.size() >= 5 && moveStr[4] == '+';
        return Move(from, to, promotion);
    }
}


} // namespace harukashogi