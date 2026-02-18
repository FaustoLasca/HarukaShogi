#ifndef SEARCH_H
#define SEARCH_H

#include <array>
#include <chrono>
#include <exception>

#include "position.h"
#include "evaluate.h"
#include "ttable.h"
#include "opening_book.h"
#include "history.h"

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
        Searcher(bool useOpeningBook = true) {
            nodeCount = 0;
            bestMove = Move::null();

            this->useOpeningBook = useOpeningBook;
            if (useOpeningBook) {
                openingBook = OpeningBook();
            }
        };

        void set_position(std::string
            sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        );

        Move search(chr::milliseconds timeLimit=chr::milliseconds(600000), int maxDepth = 20);
        std::string search(int timeLimit=600000, int maxDepth = 20);

        // search functions
        int iterative_deepening(
            chr::milliseconds timeLimit=chr::milliseconds(600000),
            int maxDepth = 20
        );
        int min_max(int depth, int ply = 0, int alpha = -INF_SCORE, int beta = INF_SCORE);
        int quiescence(int alpha, int beta);

        // for time contral
        bool is_time_up();

        // getters
        int get_node_count() const { return nodeCount; }
        Move get_best_move() const { return bestMove; }

        // utils
        void print_stats();

    private:
        Position pos;

        bool useOpeningBook;
        OpeningBook openingBook;

        TTable tt;
        HistoryEntry moveHistory[NUM_COLORS][HISTORY_SIZE];

        int generation = 0;

        Move bestMove;
        
        int nodeCount;
        
        chr::steady_clock::time_point startTime;
        chr::milliseconds timeLimit;
        bool timeUp;
};


} // namespace harukashogi

#endif // SEARCH_H