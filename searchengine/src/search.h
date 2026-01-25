#ifndef SEARCH_H
#define SEARCH_H

#include <array>

#include "position.h"
#include "evaluate.h"

namespace harukashogi {


const int MAX_DEPTH = 20;


class Searcher {
    public:
        Searcher() {
            nodeCount = 0;
            followingPV = false;
            pvTable.fill(NULL_MOVE);
            pvLength.fill(0);
        };

        int iterative_deepening(Position& pos, int depth);

        int min_max(Position& pos, int depth, int ply = 0, int alpha = -INF_SCORE, int beta = INF_SCORE);

        // getters
        int get_node_count() const { return nodeCount; }
        Move get_best_move() const { return pvTable[0]; }

    private:
        std::array<Move, MAX_DEPTH * MAX_DEPTH> pvTable;
        std::array<int, MAX_DEPTH> pvLength;
        bool followingPV;
        int nodeCount;
};


} // namespace harukashogi

#endif // SEARCH_H