#ifndef NNUE_H
#define NNUE_H

#include <cstdint>

#include "accumulator.h"
#include "layers/ft.h"
#include "layers/linear.h"
#include "../position.h"
#include "../types.h"
#include "../misc.h"

namespace harukashogi {
namespace NNUE {


constexpr size_t FEATURES = 2 * NUM_SQUARES * NUM_PIECE_TYPES + 2 * 2 * 19;
constexpr size_t ACCUMULATOR_SIZE = 16;
constexpr int Q1 = 127; // needs to fit in int8_t [-128, 127]
constexpr int Q2 = 64;  // weights need to fit in int8_t, so max weight value is  2
constexpr int SCALE = 2000; // needs to be adjusted

// define aliases for the template classes
// reduces verbosity in the code
using AccumulatorType = Accumulator<ACCUMULATOR_SIZE>;
using FeatureTransformerType = FeatureTransformer<FEATURES, ACCUMULATOR_SIZE>;


class AccumulatorStack {
    public:
        AccumulatorStack(const FeatureTransformerType& ft) : ft(ft) {}
        
        // incrementally update the accumulator and push the new accumulator to the stack
        void push(const Position& pos, Move m);
        // push a copy of the top accumulator to the stack
        void push();

        void pop();
        
        void clear() { size = 1; }

        // computes the top accumulator from scratch
        void compute(const Position& pos);

        AccumulatorType& top() { return stack[size-1]; }

    private:
        std::array<AccumulatorType, MAX_PLY+1> stack;
        int size = 1;

        const FeatureTransformerType& ft;
};


class NNUE {
    public:
        NNUE();

        // evaluate the position from the accumulator
        int32_t evaluate(const AccumulatorType& acc, Color stm) const;

        const FeatureTransformerType& feature_transformer() const { return ft; }

    private:
        FeatureTransformerType ft;
        Linear<2*ACCUMULATOR_SIZE, 1> l1;
};


} // namespace NNUE
} // namespace harukashogi

#endif // NNUE_H