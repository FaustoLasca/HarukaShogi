#include <stack>

#include "position.h"
#include "types.h"


namespace harukashogi {


constexpr size_t FEATURES = 2 * NUM_SQUARES * NUM_PIECE_TYPES + 2 * 19;
constexpr size_t ACCUMULATOR_SIZE = 8;


class NNUE {
    public:


    private:
        uint16_t l1Weights[FEATURES][ACCUMULATOR_SIZE];
        uint16_t l2Weights[ACCUMULATOR_SIZE];
};


}