#ifndef LINEAR_H
#define LINEAR_H

#include<cstddef>
#include<cstdint>
#include<cstring>

namespace harukashogi {
namespace NNUE {


template <size_t IN_SIZE, size_t OUT_SIZE>
class Linear {
    public:
        Linear() {};

        void forward(const int8_t* input, int32_t* output) const;

        const unsigned char* set_weights(const unsigned char* weights_start);

    private:
        int8_t weights[IN_SIZE][OUT_SIZE];
        int32_t bias[OUT_SIZE];
};


template <size_t IN_SIZE, size_t OUT_SIZE>
void Linear<IN_SIZE, OUT_SIZE>::forward(const int8_t* input, int32_t* output) const {
    for (size_t i = 0; i < OUT_SIZE; ++i) {
        output[i] = bias[i];
        for (size_t j = 0; j < IN_SIZE; ++j) {
            output[i] += weights[j][i] * input[j];
        }
    }
}


template <size_t IN_SIZE, size_t OUT_SIZE>
const unsigned char* Linear<IN_SIZE, OUT_SIZE>::set_weights(const unsigned char* weights_start) {
    std::memcpy(weights, weights_start, sizeof(weights));
    std::memcpy(bias, weights_start + sizeof(weights), sizeof(bias));
    return weights_start + sizeof(weights) + sizeof(bias);
}


} // namespace NNUE
} // namespace harukashogi

#endif // LINEAR_H