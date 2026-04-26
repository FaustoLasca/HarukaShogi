#ifndef CRELU_H
#define CRELU_H

#include<cstddef>
#include<cstdint>
#include<immintrin.h>

namespace harukashogi {
namespace NNUE {


template <size_t SIZE>
void crelu16(const int16_t* input, int8_t* output) {
    constexpr size_t inRegisterWidth = 256 / 16; // 16 bit elements in a 256 bit register
    constexpr size_t outRegisterWidth = 256 / 8; // 8 bit elements in a 256 bit register
    static_assert(SIZE % outRegisterWidth == 0, "Outpet must fill registers completely");
    constexpr size_t numChunks = SIZE / outRegisterWidth;

    __m256i zero = _mm256_setzero_si256();
    // used to permute the elements of the register in the final step
    const int control = 0b11011000; // permulation: [3(11), 1(01), 2(10), 0(00)]

    // for each chunk, perform the crelu and store the results in the output
    for (size_t i = 0; i < numChunks; ++i) {
        // load 32 inputs on 2 registers
        __m256i in0 = _mm256_load_si256((const __m256i*)&input[(i*2 + 0)*inRegisterWidth]);
        __m256i in1 = _mm256_load_si256((const __m256i*)&input[(i*2 + 1)*inRegisterWidth]);

        // permute the middle 64 bit groups of elements back to their original position:
        // [1, 3, 2, 4] -> [1, 2, 3, 4]
        __m256i out = _mm256_permute4x64_epi64(
            // max(element, 0) on all 8 bit elements in the register
            _mm256_max_epi8(
                // pack the 16 bit elements into 8 bit elements
                // this saturates the values to 127
                // permutes the elements:
                // [1, 2] [3, 4] -> [1, 3, 2, 4]
                _mm256_packs_epi16(in0, in1),
                zero
            ),
            control
        );

        _mm256_store_si256((__m256i*)&output[i*outRegisterWidth], out);
    }
}


} // namespace NNUE
} // namespace harukashogi

#endif // CRELU_H