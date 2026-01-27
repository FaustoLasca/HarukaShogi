#ifndef SEARCH_H
#define SEARCH_H

#include <array>
#include <chrono>
#include <exception>
#include <thread>

#include "position.h"
#include "evaluate.h"

namespace chr = std::chrono;

namespace harukashogi {


const int MAX_DEPTH = 20;


class TimeUpException : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Time limit exceeded";
        }
};


class Searcher {
    public:
        Searcher() {
            nodeCount = 0;
            followingPV = false;
            bestMove = Move::null();
            pvTable.fill(Move::null());
            pvLength.fill(0);
        };

        void set_position(std::string sfen);

        Move search(chr::milliseconds timeLimit=chr::milliseconds(600000), int maxDepth = 20);
        std::string search(int timeLimit=600000, int maxDepth = 20);

        // search functions
        int iterative_deepening(chr::milliseconds timeLimit=chr::milliseconds(600000), int maxDepth = 20);
        int min_max(int depth, int ply = 0, int alpha = -INF_SCORE, int beta = INF_SCORE);

        // for time contral
        bool is_time_up();

        // getters
        int get_node_count() const { return nodeCount; }
        Move get_best_move() const { return bestMove; }

    private:
        Position pos;
        
        std::array<Move, MAX_DEPTH * MAX_DEPTH> pvTable;
        std::array<int, MAX_DEPTH> pvLength;
        Move bestMove;
        bool followingPV;
        int nodeCount;
        
        chr::steady_clock::time_point startTime;
        chr::milliseconds timeLimit;
        bool timeUp;
};


} // namespace harukashogi

#endif // SEARCH_H