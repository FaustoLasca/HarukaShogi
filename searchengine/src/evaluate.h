#ifndef EVALUATE_H
#define EVALUATE_H

#include "position.h"

namespace harukashogi {


int evaluate(Position& pos);

int evaluate_move(const Position& pos, Move move);


constexpr int WIN_SCORE = 1000000;
constexpr int INF_SCORE = 1000001;


class ValMove : public Move {
    public:
        int value;

        void operator=(Move& move) {
            this->data = move.raw();
        };
};


} // namespace harukashogi

#endif // EVALUATE_H