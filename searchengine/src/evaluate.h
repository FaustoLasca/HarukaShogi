#ifndef EVALUATE_H
#define EVALUATE_H

#include "position.h"

namespace harukashogi {


int evaluate(Position& pos);

int evaluate_move(Position& pos, Move move);


constexpr int WIN_SCORE = 1000000;
constexpr int INF_SCORE = 1000001;


} // namespace harukashogi

#endif // EVALUATE_H