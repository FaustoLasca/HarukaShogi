#include <bitset>
#include <bit>

#include "bitboard.h"
#include "types.h"

namespace harukashogi {


int popcount(Bitboard bb) {
    return std::popcount(bb);
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