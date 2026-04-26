#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include <cstdint>
#include <cstddef>

#include "../types.h"

namespace harukashogi {
namespace NNUE {


template <size_t ACCUMULATOR_SIZE>
struct Accumulator {
    alignas(64) int16_t v[2][ACCUMULATOR_SIZE];

    int16_t* operator [] (Color c) { return v[c]; }
    const int16_t* operator [] (Color c) const { return v[c]; }
};


} // namespace NNUE
} // namespace harukashogi

#endif // ACCUMULATOR_H
