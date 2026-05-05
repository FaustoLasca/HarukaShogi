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
constexpr size_t ACCUMULATOR_SIZE = 128;
constexpr size_t H1_SIZE = 8;
constexpr size_t H2_SIZE = 32;
constexpr int Q_MULT = 127 * 64; // needs to fit in int8_t [-128, 127]
constexpr int L1_SR = 6; // scale down by 64  
constexpr int L2_SR = 6; // scale down by 64
constexpr int SCALE = 2700; // needs to be adjusted

// define aliases for the template classes
// reduces verbosity in the code
using AccumulatorType = Accumulator<ACCUMULATOR_SIZE>;
using FeatureTransformerType = FeatureTransformer<FEATURES, ACCUMULATOR_SIZE>;


class AccumulatorStack {
    public:
        AccumulatorStack(const FeatureTransformerType& ft) : ft(ft) {}
        
        // incrementally update the accumulator and push the new accumulator to the stack
        void push(MoveDiff diff);
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
        Linear<2*ACCUMULATOR_SIZE, H1_SIZE, L1_SR> l1;
        Linear<H1_SIZE, H2_SIZE, L2_SR> l2;
        Linear<H2_SIZE, 1, 0> l3;
};


} // namespace NNUE
} // namespace harukashogi

#endif // NNUE_H