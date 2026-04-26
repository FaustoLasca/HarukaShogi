#ifndef FT_H
#define FT_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <immintrin.h>

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
        void compute(const Position& pos, Accumulator<M>& acc) const;

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
        alignas(64) int16_t weights[N][M];
        alignas(64) int16_t biases[M];
};


template <size_t N, size_t M>
void FeatureTransformer<N, M>::compute(const Position& pos, Accumulator<M>& acc) const {
    constexpr size_t registerWidth = 256 / 16; // 16 bit elements in a 256 bit register
    static_assert(M % registerWidth == 0, "We must fill the registers completely");
    constexpr size_t numChunks = M / registerWidth;
    static_assert(numChunks <= 8, "Chunks will overspill the registers");
    // array of registers to accumulate the weights 16 at a time
    // one set for each perspective
    __m256i regs[2][numChunks];

    // load the biases into the registers (256 bits at a time)
    for (size_t i = 0; i < numChunks; ++i) {
        regs[BLACK][i] = _mm256_load_si256((const __m256i*)&biases[i*registerWidth]);
        regs[WHITE][i] = _mm256_load_si256((const __m256i*)&biases[i*registerWidth]);
    }

    // add the weights for the active features into the registers
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (pos.piece(sq) != NO_PIECE) {
            PieceType pt = type_of(pos.piece(sq));
            Color c = color_of(pos.piece(sq));
            size_t bIdx = board_idx<BLACK>(c, pt, sq);
            size_t wIdx = board_idx<WHITE>(c, pt, sq);


            // for each active feature, add the weight to the corresponding register
            for (size_t i = 0; i < numChunks; ++i) {
                regs[BLACK][i] = _mm256_add_epi16(
                    regs[BLACK][i],
                    _mm256_load_si256((const __m256i*)&weights[bIdx][i*registerWidth])
                );
                regs[WHITE][i] = _mm256_add_epi16(
                    regs[WHITE][i],
                    _mm256_load_si256((const __m256i*)&weights[wIdx][i*registerWidth])
                );
            }
        }
    }


    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (int count = 0; count < pos.hand_count(c, pt); ++count) {
                size_t bIdx = hand_idx<BLACK>(c, pt, count);
                size_t wIdx = hand_idx<WHITE>(c, pt, count);
                for (size_t i = 0; i < numChunks; ++i) {
                    regs[BLACK][i] = _mm256_add_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bIdx][i*registerWidth]));
                    regs[WHITE][i] = _mm256_add_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wIdx][i*registerWidth]));
                }
            }
        }
    }


    // store the results in the accumulator
    for (size_t i = 0; i < numChunks; ++i) {
        _mm256_store_si256((__m256i*)&acc[BLACK][i*registerWidth], regs[BLACK][i]);
        _mm256_store_si256((__m256i*)&acc[WHITE][i*registerWidth], regs[WHITE][i]);
    }
}


template <size_t N, size_t M>
void FeatureTransformer<N, M>::incremental_update(
    const Position& pos,
    Move m,
    const Accumulator<M>& oldAcc,
    Accumulator<M>& newAcc
) const {
    constexpr size_t registerWidth = 256 / 16; // 16 bit elements in a 256 bit register
    static_assert(M % registerWidth == 0, "We must fill the registers completely");
    constexpr size_t numChunks = M / registerWidth;
    static_assert(numChunks <= 8, "Chunks will overspill the registers");
    // array of registers to accumulate the weights 16 at a time
    // one set for each perspective
    __m256i regs[2][numChunks];

    // load the old accumulator into the registers
    for (size_t i = 0; i < numChunks; ++i) {
        regs[BLACK][i] = _mm256_load_si256((const __m256i*)&oldAcc[BLACK][i*registerWidth]);
        regs[WHITE][i] = _mm256_load_si256((const __m256i*)&oldAcc[WHITE][i*registerWidth]);
    }

    Color stm = pos.side_to_move();
    Square to = m.to();

    if (m.is_drop()) {
        PieceType pt = m.dropped();

        size_t bBoardIdx = board_idx<BLACK>(stm, pt, to);
        size_t wBoardIdx = board_idx<WHITE>(stm, pt, to);
        size_t bHandIdx = hand_idx<BLACK>(stm, pt, pos.hand_count(stm, pt)-1);
        size_t wHandIdx = hand_idx<WHITE>(stm, pt, pos.hand_count(stm, pt)-1);
        
        for (size_t i = 0; i < numChunks; ++i) {
            // add the dropped piece to the board
            regs[BLACK][i] = _mm256_add_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bBoardIdx][i*registerWidth]));
            regs[WHITE][i] = _mm256_add_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wBoardIdx][i*registerWidth]));
            // remove the dropped piece from the hand
            regs[BLACK][i] = _mm256_sub_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bHandIdx][i*registerWidth]));
            regs[WHITE][i] = _mm256_sub_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wHandIdx][i*registerWidth]));
        }
    }

    else {
        Square from = m.from();
        PieceType pt = type_of(pos.piece(from));

        size_t bFromIdx = board_idx<BLACK>(stm, pt, from);
        size_t wFromIdx = board_idx<WHITE>(stm, pt, from);
        if (m.is_promotion())
            pt = promote(pt);
        size_t bToIdx = board_idx<BLACK>(stm, pt, to);
        size_t wToIdx = board_idx<WHITE>(stm, pt, to);
        
        for (size_t i = 0; i < numChunks; ++i) {
            // remove the piece from the board
            regs[BLACK][i] = _mm256_sub_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bFromIdx][i*registerWidth]));
            regs[WHITE][i] = _mm256_sub_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wFromIdx][i*registerWidth]));
            // add the piece to the board
            regs[BLACK][i] = _mm256_add_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bToIdx][i*registerWidth]));
            regs[WHITE][i] = _mm256_add_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wToIdx][i*registerWidth]));
        }

        // if the move is a capture, remove the captured piece from the board and add it to the hand
        if (pos.is_capture(m)) {
            PieceType capturedPT = type_of(pos.piece(m.to()));

            size_t bBoardIdx = board_idx<BLACK>(~stm, capturedPT, m.to());
            size_t wBoardIdx = board_idx<WHITE>(~stm, capturedPT, m.to());
            capturedPT = unpromoted_type(capturedPT);
            int count = pos.hand_count(stm, capturedPT);
            size_t bHandIdx = hand_idx<BLACK>(stm, capturedPT, count);
            size_t wHandIdx = hand_idx<WHITE>(stm, capturedPT, count);

            for (size_t i = 0; i < numChunks; ++i) {
                // remove the captured piece from the board
                regs[BLACK][i] = _mm256_sub_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bBoardIdx][i*registerWidth]));
                regs[WHITE][i] = _mm256_sub_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wBoardIdx][i*registerWidth]));
                // add the captured piece to the hand
                regs[BLACK][i] = _mm256_add_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bHandIdx][i*registerWidth]));
                regs[WHITE][i] = _mm256_add_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wHandIdx][i*registerWidth]));
            }
        }
    }

    // store the results in the new accumulator
    for (size_t i = 0; i < numChunks; ++i) {
        _mm256_store_si256((__m256i*)&newAcc[BLACK][i*registerWidth], regs[BLACK][i]);
        _mm256_store_si256((__m256i*)&newAcc[WHITE][i*registerWidth], regs[WHITE][i]);
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