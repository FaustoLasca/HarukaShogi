#include <bitset>
#include <bit>

#include "bitboard.h"
#include "types.h"

namespace harukashogi {


int popcount(Bitboard bb) {
    return std::popcount(bb);
}

Square pop_lsb(Bitboard& bb) {
    Square sq = Square(std::countr_zero(bb));
    bb &= bb - 1;
    return sq;
}


std::ostream& operator<<(std::ostream& os, const Bitboard& bb) {
    std::bitset<9> row;
    for (int rank = R_1; rank < NUM_RANKS; ++rank) {
        row = bb >> (rank * 9) & 0x1FFull;
        for (int file = F_9; file >= F_1; --file) {
            os << row[file] << " ";
        }
        os << std::endl;
    }
    return os;
};


} // namespace harukashogi