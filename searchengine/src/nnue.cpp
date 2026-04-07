#include "nnue.h"
#include "types.h"
#include <cstring>

namespace harukashogi {
namespace NNUE {


NNUE::NNUE() {
    // TODO: temporary manual initialization of the weights.
    //       should be roughly the same as the material evaluation.
    std::memset(l1Weights, 0, sizeof(l1Weights));
    std::memset(l1Biases, 0, sizeof(l1Biases));
    std::memset(l2Weights, 0, sizeof(l2Weights));
    l2Bias = 0;

    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        l1Weights[board_idx(BLACK, PAWN, sq)][0] = 1;
        l1Weights[board_idx(WHITE, PAWN, sq)][0] = -1;

        l1Weights[board_idx(BLACK, KNIGHT, sq)][1] = 1;
        l1Weights[board_idx(WHITE, KNIGHT, sq)][1] = -1;

        l1Weights[board_idx(BLACK, LANCE, sq)][2] = 1;
        l1Weights[board_idx(WHITE, LANCE, sq)][2] = -1;

        l1Weights[board_idx(BLACK, SILVER, sq)][3] = 1;
        l1Weights[board_idx(WHITE, SILVER, sq)][3] = -1;

        l1Weights[board_idx(BLACK, GOLD, sq)][4] = 1;
        l1Weights[board_idx(WHITE, GOLD, sq)][4] = -1;
        l1Weights[board_idx(BLACK, P_PAWN, sq)][4] = 1;
        l1Weights[board_idx(WHITE, P_PAWN, sq)][4] = -1;
        l1Weights[board_idx(BLACK, P_LANCE, sq)][4] = 1;
        l1Weights[board_idx(WHITE, P_LANCE, sq)][4] = -1;
        l1Weights[board_idx(BLACK, P_KNIGHT, sq)][4] = 1;
        l1Weights[board_idx(WHITE, P_KNIGHT, sq)][4] = -1;
        l1Weights[board_idx(BLACK, P_SILVER, sq)][4] = 1;
        l1Weights[board_idx(WHITE, P_SILVER, sq)][4] = -1;

        l1Weights[board_idx(BLACK, BISHOP, sq)][5] = 1;
        l1Weights[board_idx(WHITE, BISHOP, sq)][5] = -1;

        l1Weights[board_idx(BLACK, ROOK, sq)][6] = 1;
        l1Weights[board_idx(WHITE, ROOK, sq)][6] = -1;

        l1Weights[board_idx(BLACK, P_BISHOP, sq)][7] = 1;
        l1Weights[board_idx(WHITE, P_BISHOP, sq)][7] = -1;
        l1Weights[board_idx(BLACK, P_ROOK, sq)][7] = 1;
        l1Weights[board_idx(WHITE, P_ROOK, sq)][7] = -1;
    }

    l2Weights[0] = 1;
    l2Weights[1] = 2;
    l2Weights[2] = 3;
    l2Weights[3] = 5;
    l2Weights[4] = 6;
    l2Weights[5] = 10;
    l2Weights[6] = 12;
    l2Weights[7] = 16;
}


void NNUE::compute_accumulator(Accumulator& acc, const Position& pos) const {
    // initialize the accumulator with the biases
    std::memcpy(acc.v[0], l1Biases, sizeof(l1Biases));
    std::memcpy(acc.v[1], l1Biases, sizeof(l1Biases));

    // for every board piece present add the 
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        PieceType pt = type_of(pos.piece(sq));
        Color c = color_of(pos.piece(sq));
        if (pt != NO_PIECE_TYPE) {
            for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
                acc.v[0][i] += l1Weights[board_idx(c, pt, sq)][i];
                acc.v[1][i] += l1Weights[board_idx(~c, pt, SQ_99 - sq)][i];
            }
        }
    }

    // for every hand piece add the corresponding feature weights
    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (int count = 0; count < pos.hand_count(c, pt); ++count) {
                for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
                    acc.v[0][i] += l1Weights[hand_idx(c, pt, count)][i];
                    acc.v[1][i] += l1Weights[hand_idx(~c, pt, count)][i];
                }
            }
        }
    }
}


size_t NNUE::board_idx(Color c, PieceType pt, Square sq) const {
    return c * NUM_SQUARES * NUM_PIECE_TYPES 
         + pt * NUM_SQUARES 
         + sq;
}


size_t NNUE::hand_idx(Color c, PieceType pt, int count) const {
    size_t idx;
    switch (pt) {
        case GOLD:
            idx = 0;
            break;
        case SILVER:
            idx = 4;
            break;
        case LANCE:
            idx = 8;
            break;
        case KNIGHT:
            idx = 12;
            break;
        case BISHOP:
            idx = 16;
            break;
        case ROOK:
            idx = 18;
            break;
        case PAWN:
            idx = 20;
            break;
        default:
            throw std::invalid_argument("Invalid piece type");
            break;
    }
    return idx + count;
}

}
}