#ifndef NNUE_H
#define NNUE_H

#include "../position.h"
#include "../types.h"
#include "../misc.h"
#include <cstdint>


namespace harukashogi {
namespace NNUE {


constexpr size_t FEATURES = 2 * NUM_SQUARES * NUM_PIECE_TYPES + 2 * 2 * 19;
constexpr size_t ACCUMULATOR_SIZE = 16;
constexpr int Q1 = 127; // needs to fit in int8_t [-128, 127]
constexpr int Q2 = 64;  // weights need to fit in int8_t, so max weight value is  2
constexpr int SCALE = 2000; // needs to be adjusted


struct Accumulator {
    int16_t v[2][ACCUMULATOR_SIZE];
};


struct AccumulatorStack {
    public:
        AccumulatorStack() {}

        void clear() { size = 1; }

        void push();
        void pop() { size--; }
        Accumulator& top() { return stack[size-1]; }

    private:
        std::array<Accumulator, MAX_PLY+1> stack;
        int size = 1;
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

        // compute the index of the feature given a the combination
        template <Color perspective>
        static size_t board_idx(Color c, PieceType pt, Square sq);
        template <Color perspective>
        static size_t hand_idx(Color c, PieceType pt, int count);

    private:
        int16_t l1Weights[FEATURES][ACCUMULATOR_SIZE];
        int16_t l1Biases[ACCUMULATOR_SIZE];
        
        int8_t l2Weights[2*ACCUMULATOR_SIZE];
        int32_t l2Bias;
};


} // namespace NNUE
} // namespace harukashogi

#endif // NNUE_H