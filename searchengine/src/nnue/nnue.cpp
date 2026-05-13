#include "nnue.h"
#include "../types.h"
#include "layers/crelu.h"
#include "incbin.h"

#include <cstring>

namespace harukashogi {
namespace NNUE {


// creates the following symbols:
// const unsigned char gWeightsData[];
// const unsigned char *const gWeightsEnd;
// const unsigned int gWeightsSize;
INCBIN(Weights, "../bin/nnue/9KB_acc128-32_1B.bin");
// INCBIN(Weights, "../bin/nnue/test_weights.bin");


NNUE::NNUE() {
    const unsigned char* ptr = gWeightsData;
    ptr = ft.set_weights(ptr);
    ptr = l1.set_weights(ptr);
    ptr = l2.set_weights(ptr);
    // ptr = l3.set_weights(ptr);
}


int32_t NNUE::evaluate(const AccumulatorType& acc, Color stm) const {
    // apply the feature transformer to the accumulator
    alignas(32) int8_t actAcc[2*ACCUMULATOR_SIZE];
    crelu16<ACCUMULATOR_SIZE>(acc[stm], actAcc);
    crelu16<ACCUMULATOR_SIZE>(acc[~stm], actAcc + ACCUMULATOR_SIZE);

    // apply the first linear layer to the activated accumulator
    int32_t h1[(H1_SIZE + 31) / 32 * 32];
    l1.forward(actAcc, h1);
    alignas(32) int8_t h1Act[(H1_SIZE + 31) / 32 * 32];
    crelu32<H1_SIZE>(h1, h1Act);

    // int32_t h2[(H2_SIZE + 31) / 32 * 32];
    // l2.forward(h1Act, h2);
    // alignas(32) int8_t h2Act[(H2_SIZE + 31) / 32 * 32];
    // crelu32<H2_SIZE>(h2, h2Act);
    
    // apply the linear layer to the activated hidden layer
    int32_t score;
    l2.forward(h1Act, &score);
    return (score * SCALE) / Q_MULT;
}


void AccumulatorStack::push() {
    assert(size < MAX_PLY+1);
    stack[size] = stack[size-1];
    size++;
}


void AccumulatorStack::push(Position& pos, MoveDiff diff) {
    assert(size < MAX_PLY+1);
    if (FeatureSet::requires_recompute<BLACK>(diff) 
     || FeatureSet::requires_recompute<WHITE>(diff)) {
        ft.compute(pos, stack[size]);
    }
    else {
        ft.incremental_update(pos.king_square(), diff, stack[size-1], stack[size]);
    }
    size++;
}


void AccumulatorStack::pop() {
    assert(size > 1);
    size--;
}


void AccumulatorStack::compute(const Position& pos) {
    assert(size == 1);
    ft.compute(pos, stack[0]);
}

}
}