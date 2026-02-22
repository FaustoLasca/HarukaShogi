#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <exception>

#include "position.h"
#include "evaluate.h"
#include "ttable.h"
#include "history.h"
#include "thread.h"

namespace chr = std::chrono;

namespace harukashogi {


class TimeUpException : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Time limit exceeded";
        }
};


struct SearchInfo {
    Move bestMove = Move::null();
    int eval = 0;
    int depth = 0;
    int nodeCount = 0;
};


constexpr int MAX_DEPTH = 20;


class Worker : public Thread {
    public:
        Worker(size_t id, TTable& tt) : Thread(id), tt(tt) {}

        void set_position(
            std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        );

        // struct containing the results and stats of the search
        SearchInfo info;
    private:

        void search() override;

        // iteratively performs searches at increasing depths
        void iterative_deepening();
        // the main search function
        template <bool isRoot>
        int search(int depth, int ply = 0, int alpha = -INF_SCORE, int beta = INF_SCORE);
        // quiescence search, called by the main search
        int q_search(int alpha, int beta);

        // the elements exclusive to the worker
        Position pos;

        HistoryEntry moveHistory[NUM_COLORS][HISTORY_SIZE];

        // shared elements
        TTable& tt;
};


// TODO: TEMPORARY SEARCHER CLASS
//       HAS TO BE REMOVED/REFACTORED AFTER IMPLEMENTING PARALLEL SEARCH
class Searcher {
    public:
        Searcher(bool useOpeningBook = false);

        void set_position(
            std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        );

        Move search(int timeLimit, int depth);

        void print_stats();

    private:
        bool useOpeningBook;
        std::unique_ptr<Worker> worker;
        TTable tt;
};


} // namespace harukashogi

#endif // SEARCH_H