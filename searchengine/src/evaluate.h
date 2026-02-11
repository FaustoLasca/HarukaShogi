#ifndef EVALUATE_H
#define EVALUATE_H

#include "position.h"

namespace harukashogi {


constexpr int PieceValues[NUM_PIECE_TYPES] = {
    0, 6, 5, 4, 3, 10, 12, 1,
    6, 6, 6, 16, 16, 6
};


int evaluate(Position& pos);


constexpr int WIN_SCORE = 32000;
constexpr int INF_SCORE = 32001;


} // namespace harukashogi

#endif // EVALUATE_H