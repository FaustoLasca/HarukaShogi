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
            MoveDiff diff,
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
    // number of passes needed to process all the chunks
    // (8 chunks per pass to not overspill the registers)
    constexpr size_t MAX_CHUNKS = 8;
    constexpr size_t numPasses = (numChunks + MAX_CHUNKS - 1) / MAX_CHUNKS;

    // array of registers to accumulate the weights 16 at a time
    // one set for each perspective
    __m256i regs[2][std::min(numChunks, MAX_CHUNKS)];

    for (size_t pass = 0; pass < numPasses; ++pass) {
        size_t passChunks = std::min(numChunks - pass * MAX_CHUNKS, MAX_CHUNKS);

        // load the biases into the registers (256 bits at a time)
        for (size_t i = 0; i < passChunks; ++i) {
            regs[BLACK][i] = _mm256_load_si256((const __m256i*)&biases[(pass*MAX_CHUNKS + i)*registerWidth]);
            regs[WHITE][i] = _mm256_load_si256((const __m256i*)&biases[(pass*MAX_CHUNKS + i)*registerWidth]);
        }

        // add the weights for the active features into the registers
        for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
            if (pos.piece(sq) != NO_PIECE) {
                PieceType pt = type_of(pos.piece(sq));
                Color c = color_of(pos.piece(sq));
                size_t bIdx = board_idx<BLACK>(c, pt, sq);
                size_t wIdx = board_idx<WHITE>(c, pt, sq);


                // for each active feature, add the weight to the corresponding register
                for (size_t i = 0; i < passChunks; ++i) {
                    regs[BLACK][i] = _mm256_add_epi16(
                        regs[BLACK][i],
                        _mm256_load_si256((const __m256i*)&weights[bIdx][(pass*MAX_CHUNKS + i)*registerWidth])
                    );
                    regs[WHITE][i] = _mm256_add_epi16(
                        regs[WHITE][i],
                        _mm256_load_si256((const __m256i*)&weights[wIdx][(pass*MAX_CHUNKS + i)*registerWidth])
                    );
                }
            }
        }


        for (Color c = BLACK; c < NUM_COLORS; ++c) {
            for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
                for (int count = 0; count < pos.hand_count(c, pt); ++count) {
                    size_t bIdx = hand_idx<BLACK>(c, pt, count);
                    size_t wIdx = hand_idx<WHITE>(c, pt, count);
                    for (size_t i = 0; i < passChunks; ++i) {
                        regs[BLACK][i] = _mm256_add_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
                        regs[WHITE][i] = _mm256_add_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
                    }
                }
            }
        }


        // store the results in the accumulator
        for (size_t i = 0; i < passChunks; ++i) {
            _mm256_store_si256((__m256i*)&acc[BLACK][(pass*MAX_CHUNKS + i)*registerWidth], regs[BLACK][i]);
            _mm256_store_si256((__m256i*)&acc[WHITE][(pass*MAX_CHUNKS + i)*registerWidth], regs[WHITE][i]);
        }
    }
}


template <size_t N, size_t M>
void FeatureTransformer<N, M>::incremental_update(
    MoveDiff diff,
    const Accumulator<M>& oldAcc,
    Accumulator<M>& newAcc
) const {
    constexpr size_t registerWidth = 256 / 16; // 16 bit elements in a 256 bit register
    static_assert(M % registerWidth == 0, "We must fill the registers completely");
    constexpr size_t numChunks = M / registerWidth;
    // number of passes needed to process all the chunks
    // (8 chunks per pass to not overspill the registers)
    constexpr size_t MAX_CHUNKS = 8;
    constexpr size_t numPasses = (numChunks + MAX_CHUNKS - 1) / MAX_CHUNKS;

    __m256i regs[2][std::min(numChunks, MAX_CHUNKS)];

    for (size_t pass = 0; pass < numPasses; ++pass) {
        size_t passChunks = std::min(numChunks - pass * MAX_CHUNKS, MAX_CHUNKS);
        // load the old accumulator into the registers
        for (size_t i = 0; i < passChunks; ++i) {
            regs[BLACK][i] = _mm256_load_si256((const __m256i*)&oldAcc[BLACK][(pass*MAX_CHUNKS + i)*registerWidth]);
            regs[WHITE][i] = _mm256_load_si256((const __m256i*)&oldAcc[WHITE][(pass*MAX_CHUNKS + i)*registerWidth]);
        }

        // add the moved piece to the to square feature
        size_t bIdx = board_idx<BLACK>(diff.stm, diff.toPt, diff.toSq);
        size_t wIdx = board_idx<WHITE>(diff.stm, diff.toPt, diff.toSq);
        for (size_t i = 0; i < passChunks; ++i) {
            regs[BLACK][i] = _mm256_add_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
            regs[WHITE][i] = _mm256_add_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
        }

        // remove the moved piece from the from square feature
        if (diff.fromSq != NO_SQUARE) {
            bIdx = board_idx<BLACK>(diff.stm, diff.fromPt, diff.fromSq);
            wIdx = board_idx<WHITE>(diff.stm, diff.fromPt, diff.fromSq);
        }
        else {
            bIdx = hand_idx<BLACK>(diff.stm, diff.fromPt, diff.dropCount);
            wIdx = hand_idx<WHITE>(diff.stm, diff.fromPt, diff.dropCount);
        }
        for (size_t i = 0; i < passChunks; ++i) {
            regs[BLACK][i] = _mm256_sub_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
            regs[WHITE][i] = _mm256_sub_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
        }

        // if move is a capture, remove the captured piece from the board and add it to the hand
        if (diff.capturedPt != NO_PIECE_TYPE) {
            bIdx = board_idx<BLACK>(~diff.stm, diff.capturedPt, diff.toSq);
            wIdx = board_idx<WHITE>(~diff.stm, diff.capturedPt, diff.toSq);
            PieceType unpromotedPt = unpromoted_type(diff.capturedPt);
            size_t bHandIdx = hand_idx<BLACK>(diff.stm, unpromotedPt, diff.capturedCount);
            size_t wHandIdx = hand_idx<WHITE>(diff.stm, unpromotedPt, diff.capturedCount);
            for (size_t i = 0; i < passChunks; ++i) {
                regs[BLACK][i] = _mm256_sub_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
                regs[WHITE][i] = _mm256_sub_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
                regs[BLACK][i] = _mm256_add_epi16(regs[BLACK][i], _mm256_load_si256((const __m256i*)&weights[bHandIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
                regs[WHITE][i] = _mm256_add_epi16(regs[WHITE][i], _mm256_load_si256((const __m256i*)&weights[wHandIdx][(pass*MAX_CHUNKS + i)*registerWidth]));
            }
        }

        // store the results in the new accumulator
        for (size_t i = 0; i < passChunks; ++i) {
            _mm256_store_si256((__m256i*)&newAcc[BLACK][(pass*MAX_CHUNKS + i)*registerWidth], regs[BLACK][i]);
            _mm256_store_si256((__m256i*)&newAcc[WHITE][(pass*MAX_CHUNKS + i)*registerWidth], regs[WHITE][i]);
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