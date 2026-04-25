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
INCBIN(Weights, "../bin/nnue/AdamW_acc32_21M_l5.bin");


NNUE::NNUE() {
    const unsigned char* ptr = gWeightsData;
    ptr = ft.set_weights(ptr);
    l1.set_weights(ptr);
}


int32_t NNUE::evaluate(const AccumulatorType& acc, Color stm) const {
    int8_t actAcc[2*ACCUMULATOR_SIZE];
    crelu16<ACCUMULATOR_SIZE>(acc[stm], actAcc);
    crelu16<ACCUMULATOR_SIZE>(acc[~stm], actAcc + ACCUMULATOR_SIZE);

    int32_t score;
    l1.forward(actAcc, &score);
    
    return (score * SCALE) / (Q1 * Q2);
}


void AccumulatorStack::push() {
    assert(size < MAX_PLY+1);
    stack[size] = stack[size-1];
    size++;
}


void AccumulatorStack::push(const Position& pos, Move m) {
    assert(size < MAX_PLY+1);
    ft.incremental_update(pos, m, stack[size-1], stack[size]);
    size++;
}


void AccumulatorStack::pop() {
    assert(size > 1);
    size--;
}


void AccumulatorStack::compute(const Position& pos) {
    assert(size == 1);
    ft.forward(pos, stack[0]);
}

}
}