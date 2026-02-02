#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>

#include "types.h"

namespace harukashogi {


using Bitboard = __uint128_t;

constexpr Bitboard square_bb(Square sq) {
    return static_cast<Bitboard>(1) << sq;
}

constexpr Bitboard FullBoard = (square_bb(NUM_SQUARES) - 1);
constexpr Bitboard File1BB = square_bb(SQ_11) |
                             square_bb(SQ_12) |
                             square_bb(SQ_13) |
                             square_bb(SQ_14) |
                             square_bb(SQ_15) |
                             square_bb(SQ_16) |
                             square_bb(SQ_17) |
                             square_bb(SQ_18) |
                             square_bb(SQ_19);
constexpr Bitboard File9BB = File1BB << 8;

constexpr Bitboard Rank1BB = 0x1FFull;
constexpr Bitboard Rank2BB = Rank1BB << 9;
constexpr Bitboard Rank3BB = Rank2BB << 9;
constexpr Bitboard Rank7BB = Rank1BB << 54;
constexpr Bitboard Rank8BB = Rank7BB << 9;
constexpr Bitboard Rank9BB = Rank8BB << 9;

constexpr Bitboard BlackPromZoneBB = Rank1BB | Rank2BB | Rank3BB;
constexpr Bitboard WhitePromZoneBB = Rank7BB | Rank8BB | Rank9BB;


// functions used to manipulate a bitboard
Square pop_lsb(Bitboard& bb);
int popcount(Bitboard bb);


// functions for move generation
template<Direction d>
inline Bitboard dir_attacks_bb(Bitboard bb) {
    Bitboard attacks = 0;

    // shift the bitboard in the direction
    if constexpr (dir_delta(d) > 0) {
        attacks = bb << dir_delta(d);
        attacks &= FullBoard;
    } else {
        attacks = bb >> -dir_delta(d);
    }

    // if the direction moves to the side, remove 'warp moves'
    // (going off the board to the right or left)
    if constexpr (d == E_DIR || d == NE_DIR || d == NNE_DIR || d == SE_DIR || d == SSE_DIR) {
        attacks &= ~File9BB;
    }
    else if constexpr (d == W_DIR || d == NW_DIR || d == NNW_DIR || d == SW_DIR || d == SSW_DIR) {
        attacks &= ~File1BB;
    }

    return attacks;
}


template<Piece p>
Bitboard piece_attacks_bb(Bitboard bb);

// prints a bitboard as a board of bit values
std::ostream& operator<<(std::ostream& os, const Bitboard& bb);


} // namespace harukashogi

#endif // BITBOARD_H