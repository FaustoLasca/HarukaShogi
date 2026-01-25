#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "evaluate.h"

namespace harukashogi {


class Searcher {
    public:
        Searcher() : nodeCount(0) {};

        int min_max(Position& pos, int depth, int ply = 0, int alpha = -INF_SCORE, int beta = INF_SCORE);

        int get_node_count() const { return nodeCount; }
        Move get_best_move() const { return bestMove; }

    private:
        Move bestMove;
        int nodeCount;
};


} // namespace harukashogi

#endif // SEARCH_H