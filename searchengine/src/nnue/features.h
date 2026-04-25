#ifndef FEATURES_H
#define FEATURES_H

#include "../types.h"

namespace harukashogi {
namespace NNUE {


template <Color perspective>
inline size_t board_idx(Color c, PieceType pt, Square sq) {
    if constexpr (perspective == WHITE) {
        c = ~c;
        sq = SQ_99 - sq;
    }
    return c  * size_t(NUM_SQUARES) * NUM_PIECE_TYPES 
         + pt * size_t(NUM_SQUARES) 
         + sq;
}


template <Color perspective>
inline size_t hand_idx(Color c, PieceType pt, int count) {
    if constexpr (perspective == WHITE) c = ~c;
    
    size_t idx = 2 * NUM_SQUARES * NUM_PIECE_TYPES;
    idx += (c == BLACK) ? 0 : (2*19);
    switch (pt) {
        case GOLD:
            idx += 0;
            break;
        case SILVER:
            idx += 4;
            break;
        case LANCE:
            idx += 8;
            break;
        case KNIGHT:
            idx += 12;
            break;
        case BISHOP:
            idx += 16;
            break;
        case ROOK:
            idx += 18;
            break;
        case PAWN:
            idx += 20;
            break;
        default:
            throw std::invalid_argument("Invalid piece type");
            break;
    }
    return idx + count;
}


} // namespace NNUE
} // namespace harukashogi

#endif // FEATURES_H