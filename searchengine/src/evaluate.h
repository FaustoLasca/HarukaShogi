#ifndef EVALUATE_H
#define EVALUATE_H

#include "position.h"

namespace harukashogi {


int evaluate(Position& pos);


constexpr int WIN_SCORE = 32000;
constexpr int INF_SCORE = 32001;


} // namespace harukashogi

#endif // EVALUATE_H