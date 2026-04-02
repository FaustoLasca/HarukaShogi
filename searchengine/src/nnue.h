#include <stack>

#include "position.h"
#include "types.h"


namespace harukashogi {
namespace NNUE {


constexpr size_t FEATURES = 2 * NUM_SQUARES * NUM_PIECE_TYPES + 2 * 19;
constexpr size_t ACCUMULATOR_SIZE = 8;
constexpr int16_t Q1 = 128;
constexpr int16_t Q2 = 128;
constexpr int16_t SCALE = 128*128; // needs to be adjusted


struct Accumulator {
    int16_t values[ACCUMULATOR_SIZE];
};


class NNUE {
    public:
        NNUE();

        // compute the accumulator from scratch,
        // only used for the initial position
        Accumulator compute_accumulator(const Position& pos) const;

        // update the accumulator for a given move
        // the accumulator is updated in place
        void update_accumulator(Accumulator& acc, const Position& pos, Move m) const;

        // evaluate the position from the accumulator
        int32_t evaluate(const Accumulator& acc) const;

    private:
        int16_t l1Weights[FEATURES][ACCUMULATOR_SIZE];
        int16_t l2Weights[ACCUMULATOR_SIZE];

        // compute the index of the feature given a the combination
        size_t board_idx(Color c, PieceType pt, Square sq) const;
        size_t hand_idx(Color c, PieceType pt) const;
};


} // namespace NNUE
} // namespace harukashogi