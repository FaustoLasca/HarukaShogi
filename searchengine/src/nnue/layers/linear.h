#ifndef LINEAR_H
#define LINEAR_H

#include<cstddef>
#include<cstdint>
#include<cstring>
#include<immintrin.h>

namespace harukashogi {
namespace NNUE {


template <size_t IN_SIZE, size_t OUT_SIZE>
class Linear {
    public:
        Linear() {};

        void forward(const int8_t* input, int32_t* output) const;

        const unsigned char* set_weights(const unsigned char* weights_start);

    private:
        void linear_single(const int8_t* input, int32_t* output) const;

        alignas(64) int8_t weights[OUT_SIZE*IN_SIZE];
        alignas(64) int32_t biases[OUT_SIZE];
};


template <size_t IN_SIZE, size_t OUT_SIZE>
void Linear<IN_SIZE, OUT_SIZE>::forward(const int8_t* input, int32_t* output) const {
    linear_single(input, output);
}


// function to perform a linear transformation on a single output
template <size_t IN_SIZE, size_t OUT_SIZE>
void Linear<IN_SIZE, OUT_SIZE>::linear_single(const int8_t* input, int32_t* output) const {
    static_assert(OUT_SIZE == 1);
    constexpr size_t inRegisterWidth = 256 / 8; // 8 bit elements in a 256 bit register
    static_assert(IN_SIZE % inRegisterWidth == 0, "Input must fill registers completely");
    constexpr size_t numChunks = IN_SIZE / inRegisterWidth;

    // initialize the accumulator to zero
    __m256i acc = _mm256_setzero_si256();
    __m256i one = _mm256_set1_epi16(1);

    for (size_t i = 0; i < numChunks; ++i) {
        // load an input chunk into a register (32 elements)
        __m256i in = _mm256_load_si256((const __m256i*)&input[i*inRegisterWidth]);

        // multiply the input chunk by the weights and accumulate the results:
        // 1 - dot product between 8 bit elements and sum pairs into 16bit elements
        __m256i prod = _mm256_maddubs_epi16(in, _mm256_load_si256((const __m256i*)&weights[i*inRegisterWidth]));
        // 2 - sum pairs of 16 bit elements into 32 bit elements (mult by one and sum in pairs)
        prod = _mm256_madd_epi16(prod, one);
        // 3 - add the results to the accumulator
        acc = _mm256_add_epi32(acc, prod);
    }

    // sum the 8 32 bit elements in the accumulator into a single 32 bit element
    // 1 - divide the accumulator into two 128 bit elements and add them together
    __m128i accHi = _mm256_extracti128_si256(acc, 1);
    __m128i accLo = _mm256_castsi256_si128(acc);
    __m128i sum128 = _mm_add_epi32(accHi, accLo);
    // 2 - fold 128 bits: add the high and low 64 bits together
    __m128i hi64 = _mm_unpackhi_epi64(sum128, sum128);
    sum128 = _mm_add_epi32(sum128, hi64);
    // 3 - final fold: add the 2 remaining 32 bit elements together
    __m128i hi32 = _mm_shuffle_epi32(sum128, _MM_SHUFFLE(0, 0, 0, 1));
    sum128 = _mm_add_epi32(sum128, hi32);
    // 4 - extract the final 32 bit element and add the bias
    *output = _mm_cvtsi128_si32(sum128) + biases[0];
}


template <size_t IN_SIZE, size_t OUT_SIZE>
const unsigned char* Linear<IN_SIZE, OUT_SIZE>::set_weights(const unsigned char* weights_start) {
    std::memcpy(weights, weights_start, sizeof(weights));
    std::memcpy(biases, weights_start + sizeof(weights), sizeof(biases));
    return weights_start + sizeof(weights) + sizeof(biases);
}


} // namespace NNUE
} // namespace harukashogi

#endif // LINEAR_H