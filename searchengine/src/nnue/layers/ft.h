#ifndef FT_H
#define FT_H

#include <cstdint>
#include <cstddef>
#include <cstring>

#include "../features.h"
#include "../accumulator.h"
#include "../../position.h"
#include "../../types.h"

namespace harukashogi {
namespace NNUE {


template <size_t N, size_t M>
class FeatureTransformer {
    public:
        FeatureTransformer() {}

        // compute the accumulator from scratch
        void forward(const Position& pos, Accumulator<M>& acc) const;

        // update the accumulator incrementally
        void incremental_update(
            const Position& pos,
            Move m,
            const Accumulator<M>& oldAcc,
            Accumulator<M>& newAcc
        ) const;

        // set the weights and biases
        const unsigned char* set_weights(const unsigned char* weights_start);

    private:
        int16_t weights[N][M];
        int16_t biases[M];
};


template <size_t N, size_t M>
void FeatureTransformer<N, M>::forward(const Position& pos, Accumulator<M>& acc) const {
    // initialize the accumulator with the biases
    std::memcpy(acc[BLACK], biases, sizeof(biases));
    std::memcpy(acc[WHITE], biases, sizeof(biases));

    // for every board piece present add the 
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (pos.piece(sq) != NO_PIECE) {
            PieceType pt = type_of(pos.piece(sq));
            Color c = color_of(pos.piece(sq));
            for (int i = 0; i < M; ++i) {
                acc[BLACK][i] += weights[board_idx<BLACK>(c, pt, sq)][i];
                acc[WHITE][i] += weights[board_idx<WHITE>(c, pt, sq)][i];
            }
        }
    }

    // for every hand piece add the corresponding feature weights
    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (int count = 0; count < pos.hand_count(c, pt); ++count) {
                for (int i = 0; i < M; ++i) {
                    acc[BLACK][i] += weights[hand_idx<BLACK>(c, pt, count)][i];
                    acc[WHITE][i] += weights[hand_idx<WHITE>(c, pt, count)][i];
                }
            }
        }
    }
}


template <size_t N, size_t M>
void FeatureTransformer<N, M>::incremental_update(
    const Position& pos,
    Move m,
    const Accumulator<M>& oldAcc,
    Accumulator<M>& newAcc
) const {
    // copy the old accumulator to the new accumulator
    newAcc = oldAcc;

    Color stm = pos.side_to_move();
    Square to = m.to();

    if (m.is_drop()) {
        PieceType pt = m.dropped();
        // add the dropped piece to the board
        for (int i = 0; i < M; ++i) {
            newAcc[BLACK][i] += weights[board_idx<BLACK>(stm, pt, to)][i];
            newAcc[WHITE][i] += weights[board_idx<WHITE>(stm, pt, to)][i];
        }
        // remove the dropped piece from the hand
        for (int i = 0; i < M; ++i) {
            newAcc[BLACK][i] -= weights[hand_idx<BLACK>(stm, pt, pos.hand_count(stm, pt)-1)][i];
            newAcc[WHITE][i] -= weights[hand_idx<WHITE>(stm, pt, pos.hand_count(stm, pt)-1)][i];
        }
    }

    else {
        Square from = m.from();
        PieceType pt = type_of(pos.piece(from));
        // remove the piece from the board
        for (int i = 0; i < M; ++i) {
            newAcc[BLACK][i] -= weights[board_idx<BLACK>(stm, pt, from)][i];
            newAcc[WHITE][i] -= weights[board_idx<WHITE>(stm, pt, from)][i];
        }
        // add the piece to the board
        if (m.is_promotion())
            pt = promote(pt);
        for (int i = 0; i < M; ++i) {
            newAcc[BLACK][i] += weights[board_idx<BLACK>(stm, pt, to)][i];
            newAcc[WHITE][i] += weights[board_idx<WHITE>(stm, pt, to)][i];
        }

        // if the move is a capture, remove the captured piece from the board and add it to the hand
        if (pos.is_capture(m)) {
            // remove the captured piece from the board
            PieceType capturedPT = type_of(pos.piece(m.to()));
            for (int i = 0; i < M; ++i) {
                newAcc[BLACK][i] -= weights[board_idx<BLACK>(~stm, capturedPT, m.to())][i];
                newAcc[WHITE][i] -= weights[board_idx<WHITE>(~stm, capturedPT, m.to())][i];
            }
            // add the captured piece to the hand
            capturedPT = unpromoted_type(capturedPT);
            int count = pos.hand_count(stm, capturedPT);
            for (int i = 0; i < M; ++i) {
                newAcc[BLACK][i] += weights[hand_idx<BLACK>(stm, capturedPT, count)][i];
                newAcc[WHITE][i] += weights[hand_idx<WHITE>(stm, capturedPT, count)][i];
            }
        }
    }
}


template <size_t N, size_t M>
const unsigned char* FeatureTransformer<N, M>::set_weights(const unsigned char* weightsStart) {
    std::memcpy(this->weights, weightsStart, sizeof(this->weights));
    std::memcpy(this->biases, weightsStart + sizeof(weights), sizeof(this->biases));
    return weightsStart + sizeof(weights) + sizeof(biases);
}


} // namespace NNUE
} // namespace harukashogi

#endif