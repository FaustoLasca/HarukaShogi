#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>

#include "types.h"

namespace harukashogi {

// 128-bit bitboard type
using Bitboard = __uint128_t;


namespace Bitboards {
// initializes the precomputed data structures for bitboards
void init();
}


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

constexpr Bitboard WLastRankBB = Rank9BB;
constexpr Bitboard WSecondLastRankBB = Rank8BB;
constexpr Bitboard BLastRankBB = Rank1BB;
constexpr Bitboard BSecondLastRankBB = Rank2BB;


constexpr Bitboard invert(Bitboard bb) {
    return ~bb & FullBoard;
}


// returns the bitboard of all the squares between two squares
// used for check evasions and pins
Bitboard between_bb(Square from, Square to);


// functions used to manipulate a bitboard
Square lsb(Bitboard bb);
Square pop_lsb(Bitboard& bb);
int popcount(Bitboard bb);
bool one_bit(Bitboard bb);


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
Bitboard dir_attacks_bb(Bitboard from, Direction d);


Bitboard dir_attacks_bb(Square from, Color c, PieceType pt);
template<Color c, PieceType pt>
inline Bitboard dir_attacks_bb(Square from) {
    return dir_attacks_bb(from, c, pt);
}


template<Color c, PieceType pt>
inline Bitboard sliding_attacks_bb(Square from, Bitboard occupied) {
    Bitboard bb;
    Direction d;
    Bitboard attacks = 0;

    constexpr size_t index = sl_dir_index(make_piece(c, pt));

    for (size_t i=0; i<4 && PSlidingDirections[index][i] != NULL_DIR; ++i) {
        d = PSlidingDirections[index][i];
        bb = square_bb(from);

        while (bb) {
            // TODO: THIS IS HORRIFYING, CHECK FOR A BETTER COMPILE TIME SOLUTION
            switch (d) {
                case N_DIR:
                    bb = dir_attacks_bb<N_DIR>(bb);
                    break;
                case NE_DIR:
                    bb = dir_attacks_bb<NE_DIR>(bb);
                    break;
                case E_DIR:
                    bb = dir_attacks_bb<E_DIR>(bb);
                    break;
                case SE_DIR:
                    bb = dir_attacks_bb<SE_DIR>(bb);
                    break;
                case S_DIR:
                    bb = dir_attacks_bb<S_DIR>(bb);
                    break;
                case SW_DIR:
                    bb = dir_attacks_bb<SW_DIR>(bb);
                    break;
                case W_DIR:
                    bb = dir_attacks_bb<W_DIR>(bb);
                    break;
                case NW_DIR:
                    bb = dir_attacks_bb<NW_DIR>(bb);
                default:
                    break;
            }
            
            attacks |= bb;
            bb &= ~occupied;
        }
    }

    return attacks;
}


template<Color c, PieceType pt>
inline Bitboard attacks_bb(Square from, Bitboard occupied = 0) {
    Bitboard attacks = dir_attacks_bb<c, pt>(from);
    if constexpr (sliding_type_index(pt) != -1) {
        attacks |= sliding_attacks_bb<c, pt>(from, occupied);
    }
    return attacks;
}


// prints a bitboard as a board of bit values
std::ostream& operator<<(std::ostream& os, const Bitboard& bb);


} // namespace harukashogi

#endif // BITBOARD_H