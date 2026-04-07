#include "nnue.h"
#include "types.h"
#include <cstring>
#include <algorithm>

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

    // hand piece weights
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(BLACK, GOLD, i)][4] = 1;
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(WHITE, GOLD, i)][4] = -1;
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(BLACK, SILVER, i)][3] = 1;
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(WHITE, SILVER, i)][3] = -1;
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(BLACK, LANCE, i)][2] = 1;
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(WHITE, LANCE, i)][2] = -1;
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(BLACK, KNIGHT, i)][1] = 1;
    for (int i = 0; i < 4; ++i) l1Weights[hand_idx(WHITE, KNIGHT, i)][1] = -1;
    for (int i = 0; i < 2; ++i) l1Weights[hand_idx(BLACK, BISHOP, i)][5] = 1;
    for (int i = 0; i < 2; ++i) l1Weights[hand_idx(WHITE, BISHOP, i)][5] = -1;
    for (int i = 0; i < 2; ++i) l1Weights[hand_idx(BLACK, ROOK, i)][6] = 1;
    for (int i = 0; i < 2; ++i) l1Weights[hand_idx(WHITE, ROOK, i)][6] = -1;
    for (int i = 0; i < 18; ++i) l1Weights[hand_idx(BLACK, PAWN, i)][0] = 1;
    for (int i = 0; i < 18; ++i) l1Weights[hand_idx(WHITE, PAWN, i)][0] = -1;

    l2Weights[0] = 1;
    l2Weights[1] = 3;
    l2Weights[2] = 4;
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
        if (pos.piece(sq) != NO_PIECE) {
            PieceType pt = type_of(pos.piece(sq));
            Color c = color_of(pos.piece(sq));
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


void NNUE::update_accumulator(Accumulator& acc, const Position& pos, Move m) const {
    Color stm = pos.side_to_move();
    Square to = m.to();

    if (m.is_drop()) {
        PieceType pt = m.dropped();
        // add the dropped piece to the board
        for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
            acc.v[0][i] += l1Weights[board_idx(stm, pt, to)][i];
            acc.v[1][i] += l1Weights[board_idx(~stm, pt, SQ_99 - to)][i];
        }
        // remove the dropped piece from the hand
        for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
            acc.v[0][i] -= l1Weights[hand_idx(stm, pt, pos.hand_count(stm, pt)-1)][i];
            acc.v[1][i] -= l1Weights[hand_idx(~stm, pt, pos.hand_count(stm, pt)-1)][i];
        }
    }

    else {
        Square from = m.from();
        PieceType pt = type_of(pos.piece(from));
        // remove the piece from the board
        for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
            acc.v[0][i] -= l1Weights[board_idx(stm, pt, from)][i];
            acc.v[1][i] -= l1Weights[board_idx(~stm, pt, SQ_99 - from)][i];
        }
        // add the piece to the board
        if (m.is_promotion())
            pt = promote(pt);
        for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
            acc.v[0][i] += l1Weights[board_idx(stm, pt, to)][i];
            acc.v[1][i] += l1Weights[board_idx(~stm, pt, SQ_99 - to)][i];
        }

        // if the move is a capture, remove the captured piece from the board and add it to the hand
        if (pos.is_capture(m)) {
            // remove the captured piece from the board
            PieceType capturedPT = type_of(pos.piece(m.to()));
            for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
                acc.v[0][i] -= l1Weights[board_idx(~stm, capturedPT, m.to())][i];
                acc.v[1][i] -= l1Weights[board_idx(stm, capturedPT, SQ_99 - m.to())][i];
            }
            // add the captured piece to the hand
            capturedPT = unpromoted_type(capturedPT);
            for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
                acc.v[0][i] += l1Weights[hand_idx(stm, capturedPT, pos.hand_count(stm, capturedPT))][i];
                acc.v[1][i] += l1Weights[hand_idx(~stm, capturedPT, pos.hand_count(stm, capturedPT))][i];
            }
        }
    }
}


int32_t NNUE::evaluate(const Accumulator& acc, Color stm) const {
    int8_t actAcc[2*ACCUMULATOR_SIZE];

    for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
        actAcc[i]                    = static_cast<int8_t>(std::clamp<int16_t>(acc.v[stm][i], 0, 127));
        actAcc[ACCUMULATOR_SIZE + i] = static_cast<int8_t>(std::clamp<int16_t>(acc.v[~stm][i], 0, 127));
    }

    int32_t score = l2Bias;
    for (int i = 0; i < 2*ACCUMULATOR_SIZE; ++i) {
        score += l2Weights[i] * actAcc[i];
    }
    return (score * SCALE) / (Q1 * Q2);
}


size_t NNUE::board_idx(Color c, PieceType pt, Square sq) const {
    return c * NUM_SQUARES * NUM_PIECE_TYPES 
         + pt * NUM_SQUARES 
         + sq;
}


size_t NNUE::hand_idx(Color c, PieceType pt, int count) const {
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

}
}