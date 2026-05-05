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
INCBIN(Weights, "../bin/nnue/AdamW_acc128-8-32_1B.bin");
// INCBIN(Weights, "../bin/nnue/test_weights.bin");


NNUE::NNUE() {
    const unsigned char* ptr = gWeightsData;
    ptr = ft.set_weights(ptr);
    ptr = l1.set_weights(ptr);
    ptr = l2.set_weights(ptr);
    ptr = l3.set_weights(ptr);
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

    int32_t h2[32];
    l2.forward(h1Act, h2);
    alignas(32) int8_t h2Act[32];
    crelu32<32>(h2, h2Act);
    
    // apply the linear layer to the activated hidden layer
    int32_t score;
    l3.forward(h2Act, &score);
    return (score * SCALE) / (Q0 * Q1);
}


void AccumulatorStack::push() {
    assert(size < MAX_PLY+1);
    stack[size] = stack[size-1];
    size++;
}


void AccumulatorStack::push(MoveDiff diff) {
    assert(size < MAX_PLY+1);
    ft.incremental_update(diff, stack[size-1], stack[size]);
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