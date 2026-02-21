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


constexpr int MAX_DEPTH = 20;


class Worker : public Thread {
    public:
        Worker(size_t id, TTable& tt) : Thread(id), tt(tt) {}

        void set_position(
            std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        );

        // TODO: temporarily public for testing
        // the results of the search
        Move bestMove = Move::null();
        int eval = 0;
        int depth = 0;

        // stats
        int nodeCount = 0;

        // TODO: TEMPORARY TIME CONTROL LOGIC
        //       WILL NOT BE HANDLED IN THE WORKER
        chr::steady_clock::time_point startTime;
        chr::milliseconds timeLimit;
        bool timeUp;
    private:

        void search() override;

        // iteratively performs searches at increasing depths
        void iterative_deepening();
        // the main search function
        template <bool isRoot>
        int search(int depth, int ply = 0, int alpha = -INF_SCORE, int beta = INF_SCORE);
        // quiescence search, called by the main search
        int q_search(int alpha, int beta);

        // function used to abord the search
        void stop_check();

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