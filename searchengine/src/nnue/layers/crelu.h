#ifndef CRELU_H
#define CRELU_H

#include<cstddef>
#include<cstdint>
#include<algorithm>

namespace harukashogi {
namespace NNUE {


template <size_t SIZE>
void crelu16(const int16_t* input, int8_t* output) {
    for (size_t i = 0; i < SIZE; ++i) {
        output[i] = std::clamp<int16_t>(input[i], 0, 127);
    }
}


} // namespace NNUE
} // namespace harukashogi

#endif // CRELU_H