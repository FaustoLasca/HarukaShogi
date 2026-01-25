#ifndef EVALUATE_H
#define EVALUATE_H

#include "position.h"

namespace harukashogi {


int evaluate(Position& pos);


constexpr int WIN_SCORE = 100000;
constexpr int INF_SCORE = 100001;


} // namespace harukashogi

#endif // EVALUATE_H