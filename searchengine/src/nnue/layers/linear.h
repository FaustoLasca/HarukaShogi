#ifndef LINEAR_H
#define LINEAR_H

#include<cstddef>
#include<cstdint>
#include<cstring>
#include<immintrin.h>

namespace harukashogi {
namespace NNUE {


template <size_t IN_SIZE, size_t OUT_SIZE, int SR_BITS>
class Linear {
    public:
        Linear() {};

        void forward(const int8_t* input, int32_t* output) const requires (OUT_SIZE > 1);
        void forward(const int8_t* input, int32_t* output) const requires (OUT_SIZE == 1);

        const unsigned char* set_weights(const unsigned char* weights_start);

    private:
        void linear_single(const int8_t* input, int32_t* output) const;

        static constexpr size_t inWeightsSize = ((IN_SIZE + 31) / 32) * 32;
        alignas(64) int8_t weights[OUT_SIZE*inWeightsSize];
        alignas(64) int32_t biases[OUT_SIZE];
};


inline void m256_add_dpbus_epi32(__m256i& acc, __m256i a, __m256i b) {
    // multiply the input chunk by the weights and accumulate the results:
    // 1 - dot product between 8 bit elements and sum pairs into 16bit elements
    __m256i prod = _mm256_maddubs_epi16(a, b);
    // 2 - sum pairs of 16 bit elements into 32 bit elements (mult by one and sum in pairs)
    __m256i one = _mm256_set1_epi16(1);
    prod = _mm256_madd_epi16(prod, one);
    // 3 - add the results to the accumulator
    acc = _mm256_add_epi32(acc, prod);
}


inline __m128i m256_haddx4(__m256i a, __m256i b, __m256i c, __m256i d) {
    // add pairs horizontally in each 256 bit register and put the results in a
    // results are ordered as follows:
    // [a0+a1, a2+a3, b0+b1, b2+b3, a4+a5, a6+a7, b4+b5, b6+b7]
    // same goes for c and d
    a = _mm256_hadd_epi32(a, b);
    c = _mm256_hadd_epi32(c, d);

    // same operation again on the results of the prevoius, resulting in:
    // [a0+...+a3, b0+...+b3, c0+...+c3, d0+...+d3, a4+...+a7, b4+...+b7, c4+...+c7, d4+...+d7]
    a = _mm256_hadd_epi32(a, c);

    // extract the lower and highr 128 bits and sum them for the final result
    __m128i lo = _mm256_castsi256_si128(a);
    __m128i hi = _mm256_extracti128_si256(a, 1);
    return _mm_add_epi32(lo, hi);
}


template <size_t IN_SIZE, size_t OUT_SIZE, int SR_BITS>
void Linear<IN_SIZE, OUT_SIZE, SR_BITS>::forward(const int8_t* input, int32_t* output) const 
requires (OUT_SIZE > 1)
{
    constexpr size_t inRegisterWidth = 256 / 8; // 8 bit elements in a 256 bit register
    static_assert(OUT_SIZE % 4 == 0, "Output aize must be a multiple of 4");
    constexpr size_t numInChunks = (IN_SIZE + inRegisterWidth - 1) / inRegisterWidth;
    constexpr size_t numOutChunks = OUT_SIZE / 4; // compute 4 output elements at a time

    for (size_t i = 0; i < numOutChunks; ++i) {
        // offsets for the 4 output elements' weights
        const size_t outOffset0 = (i * 4 + 0)*IN_SIZE;
        const size_t outOffset1 = (i * 4 + 1)*IN_SIZE;
        const size_t outOffset2 = (i * 4 + 2)*IN_SIZE;
        const size_t outOffset3 = (i * 4 + 3)*IN_SIZE;

        // initialize 4 accumulators to zero, bias added at the end
        __m256i acc0 = _mm256_setzero_si256();
        __m256i acc1 = _mm256_setzero_si256();
        __m256i acc2 = _mm256_setzero_si256();
        __m256i acc3 = _mm256_setzero_si256();

        for (size_t j = 0; j < numInChunks; ++j) {
            // input register is reused 4 times
            const __m256i in = _mm256_load_si256((const __m256i*)&input[j*inRegisterWidth]);

            // multiply the input chunk by the weights and accumulate the results
            m256_add_dpbus_epi32(acc0, in, 
                _mm256_load_si256((const __m256i*)&weights[outOffset0 + j * inRegisterWidth]));
            m256_add_dpbus_epi32(acc1, in, 
                _mm256_load_si256((const __m256i*)&weights[outOffset1 + j * inRegisterWidth]));
            m256_add_dpbus_epi32(acc2, in, 
                _mm256_load_si256((const __m256i*)&weights[outOffset2 + j * inRegisterWidth]));
            m256_add_dpbus_epi32(acc3, in, 
                _mm256_load_si256((const __m256i*)&weights[outOffset3 + j * inRegisterWidth]));
        }
        
        // add the 8 values in each register horizontally, returning a 128bit register with the
        // 4 int32 sums
        __m128i out = m256_haddx4(acc0, acc1, acc2, acc3);
        
        // add the bias to the results
        const __m128i bias = _mm_load_si128((const __m128i*)&biases[i * 4]);
        out = _mm_add_epi32(out, bias);

        // if necessary, resize the results
        if constexpr (SR_BITS != 0) {
            out = _mm_srai_epi32(out, SR_BITS);
        }

        // store the results
        _mm_store_si128((__m128i*)&output[i * 4], out);
    }
}


// function to perform a linear transformation on a single output
template <size_t IN_SIZE, size_t OUT_SIZE, int SR_BITS>
void Linear<IN_SIZE, OUT_SIZE, SR_BITS>::forward(const int8_t* input, int32_t* output) const
requires (OUT_SIZE == 1)
{
    constexpr size_t inRegisterWidth = 256 / 8; // 8 bit elements in a 256 bit register
    constexpr size_t numInChunks = (IN_SIZE + inRegisterWidth - 1) / inRegisterWidth;

    // initialize the accumulator to zero
    __m256i acc = _mm256_setzero_si256();

    for (size_t i = 0; i < numInChunks; ++i) {
        // load an input chunk into a register (32 elements)
        const __m256i in = _mm256_load_si256((const __m256i*)&input[i*inRegisterWidth]);

        // multiply the input chunk by the weights and accumulate the results
        m256_add_dpbus_epi32(acc,in,_mm256_load_si256((const __m256i*)&weights[i*inRegisterWidth]));
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
    *output = (_mm_cvtsi128_si32(sum128) + biases[0]) >> SR_BITS;
}


template <size_t IN_SIZE, size_t OUT_SIZE, int SR_BITS>
const unsigned char* Linear<IN_SIZE, OUT_SIZE, SR_BITS>::set_weights(const unsigned char* weights_start) {
    // initialize the weights array to zero, then copy the weights into the array
    // this adds 0 padding to the weights to fill the registers completely during inference
    std::memset(weights, 0, sizeof(weights));
    for (size_t i = 0; i < OUT_SIZE; ++i) {
        std::memcpy(&weights[i*inWeightsSize], weights_start + i*IN_SIZE, IN_SIZE);
    }
    std::memcpy(biases, weights_start + OUT_SIZE*IN_SIZE, sizeof(biases));
    return weights_start + sizeof(weights) + sizeof(biases);
}


} // namespace NNUE
} // namespace harukashogi

#endif // LINEAR_H