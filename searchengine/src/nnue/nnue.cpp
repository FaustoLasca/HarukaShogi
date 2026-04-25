#include "nnue.h"
#include "../types.h"
#include "incbin.h"

#include <cstring>
#include <algorithm>

namespace harukashogi {
namespace NNUE {


void AccumulatorStack::push() {
    stack[size] = stack[size-1];
    size++;
}


// creates the following symbols:
// const unsigned char gWeightsData[];
// const unsigned char *const gWeightsEnd;
// const unsigned int gWeightsSize;
INCBIN(Weights, "../bin/nnue/AdamW_acc16_20M.bin");


NNUE::NNUE() {
    const unsigned char* l1wStart = gWeightsData;
    const unsigned char* l1bStart = l1wStart + sizeof(l1Weights);
    const unsigned char* l2wStart = l1bStart + sizeof(l1Biases);
    const unsigned char* l2bStart = l2wStart + sizeof(l2Weights);

    std::memcpy(l1Weights, l1wStart, sizeof(l1Weights));
    std::memcpy(l1Biases, l1bStart, sizeof(l1Biases));
    std::memcpy(l2Weights, l2wStart, sizeof(l2Weights));
    std::memcpy(&l2Bias, l2bStart, sizeof(l2Bias));
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
                acc.v[0][i] += l1Weights[board_idx<BLACK>(c, pt, sq)][i];
                acc.v[1][i] += l1Weights[board_idx<WHITE>(c, pt, sq)][i];
            }
        }
    }

    // for every hand piece add the corresponding feature weights
    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (int count = 0; count < pos.hand_count(c, pt); ++count) {
                for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
                    acc.v[0][i] += l1Weights[hand_idx<BLACK>(c, pt, count)][i];
                    acc.v[1][i] += l1Weights[hand_idx<WHITE>(c, pt, count)][i];
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
            acc.v[0][i] += l1Weights[board_idx<BLACK>(stm, pt, to)][i];
            acc.v[1][i] += l1Weights[board_idx<WHITE>(stm, pt, to)][i];
        }
        // remove the dropped piece from the hand
        for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
            acc.v[0][i] -= l1Weights[hand_idx<BLACK>(stm, pt, pos.hand_count(stm, pt)-1)][i];
            acc.v[1][i] -= l1Weights[hand_idx<WHITE>(stm, pt, pos.hand_count(stm, pt)-1)][i];
        }
    }

    else {
        Square from = m.from();
        PieceType pt = type_of(pos.piece(from));
        // remove the piece from the board
        for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
            acc.v[0][i] -= l1Weights[board_idx<BLACK>(stm, pt, from)][i];
            acc.v[1][i] -= l1Weights[board_idx<WHITE>(stm, pt, from)][i];
        }
        // add the piece to the board
        if (m.is_promotion())
            pt = promote(pt);
        for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
            acc.v[0][i] += l1Weights[board_idx<BLACK>(stm, pt, to)][i];
            acc.v[1][i] += l1Weights[board_idx<WHITE>(stm, pt, to)][i];
        }

        // if the move is a capture, remove the captured piece from the board and add it to the hand
        if (pos.is_capture(m)) {
            // remove the captured piece from the board
            PieceType capturedPT = type_of(pos.piece(m.to()));
            for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
                acc.v[0][i] -= l1Weights[board_idx<BLACK>(~stm, capturedPT, m.to())][i];
                acc.v[1][i] -= l1Weights[board_idx<WHITE>(~stm, capturedPT, m.to())][i];
            }
            // add the captured piece to the hand
            capturedPT = unpromoted_type(capturedPT);
            int count = pos.hand_count(stm, capturedPT);
            for (int i = 0; i < ACCUMULATOR_SIZE; ++i) {
                acc.v[0][i] += l1Weights[hand_idx<BLACK>(stm, capturedPT, count)][i];
                acc.v[1][i] += l1Weights[hand_idx<WHITE>(stm, capturedPT, count)][i];
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


template <Color perspective>
size_t NNUE::board_idx(Color c, PieceType pt, Square sq) {
    if constexpr (perspective == WHITE) {
        c = ~c;
        sq = SQ_99 - sq;
    }
    return c * NUM_SQUARES * NUM_PIECE_TYPES 
         + pt * NUM_SQUARES 
         + sq;
}
template size_t NNUE::board_idx<BLACK>(Color c, PieceType pt, Square sq);
template size_t NNUE::board_idx<WHITE>(Color c, PieceType pt, Square sq);


template <Color perspective>
size_t NNUE::hand_idx(Color c, PieceType pt, int count) {
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
template size_t NNUE::hand_idx<BLACK>(Color c, PieceType pt, int count);
template size_t NNUE::hand_idx<WHITE>(Color c, PieceType pt, int count);


}
}