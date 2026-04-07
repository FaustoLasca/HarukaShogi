#include "position.h"
#include "types.h"
#include <cstdint>


namespace harukashogi {
namespace NNUE {


constexpr size_t FEATURES = 2 * NUM_SQUARES * NUM_PIECE_TYPES + 2 * 2 * 19;
constexpr size_t ACCUMULATOR_SIZE = 8;
constexpr int Q1 = 127; // needs to fit in int8_t [-128, 127]
constexpr int Q2 = 64;  // weights need to fit in int8_t, so max weight value is  2
constexpr int SCALE = 100 * 127 * 64; // needs to be adjusted


struct Accumulator {
    int16_t v[2][ACCUMULATOR_SIZE];
};


class NNUE {
    public:
        NNUE();

        // compute the accumulator from scratch,
        // only used for the initial position
        void compute_accumulator(Accumulator& acc, const Position& pos) const;

        // update the accumulator for a given move
        void update_accumulator(Accumulator& acc, const Position& pos, Move m) const;

        // evaluate the position from the accumulator
        int32_t evaluate(const Accumulator& acc, Color stm) const;

    private:
        int16_t l1Weights[FEATURES][ACCUMULATOR_SIZE];
        int16_t l1Biases[ACCUMULATOR_SIZE];
        int8_t l2Weights[2*ACCUMULATOR_SIZE];
        int8_t l2Bias;

        // compute the index of the feature given a the combination
        size_t board_idx(Color c, PieceType pt, Square sq) const;
        size_t hand_idx(Color c, PieceType pt, int count) const;
};


} // namespace NNUE
} // namespace harukashogi