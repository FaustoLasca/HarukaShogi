#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <exception>

#include "position.h"
#include "evaluate.h"
#include "ttable.h"
#include "history.h"
#include "thread.h"
#include "opening_book.h"
#include "types.h"

namespace chr = std::chrono;

namespace harukashogi {


class AbortSearchException : public std::exception {
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
    chr::time_point<chr::steady_clock> startTime = chr::steady_clock::now();
};


struct SearchLimits {
    SearchLimits() : 
        startTime(chr::steady_clock::now()),
        time{chr::milliseconds(0), chr::milliseconds(0)},
        inc{chr::milliseconds(0), chr::milliseconds(0)},
        byoyomi(0), moveTime(0),
        depth(0), nodes(0), movesToGo(0),
        infinite(false), ponder(false) {}

    chr::time_point<chr::steady_clock> startTime;
    chr::milliseconds time[NUM_COLORS], inc[NUM_COLORS], byoyomi, moveTime;
    int depth, nodes, movesToGo;
    bool infinite, ponder;
};


class OutputManager {
    public:
        virtual void on_best_move(Move bestMove, Move ponderMove) = 0;
        virtual void on_iter(const SearchInfo& info) = 0;
};


constexpr int MAX_DEPTH = 20;


class Worker : public Thread {
    public:
        Worker(size_t id, TTable& tt, ThreadPool<Worker>& threads, OutputManager& outputManager) : 
            Thread(id),
            tt(tt),
            threads(threads),
            outputManager(outputManager) {}

        void set_position(
            std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        );

        void set_limits(const SearchLimits& limits) {
            // only the master thread uses the limits
            assert(is_master());
            this->limits = limits;
        };

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
        int q_search(int alpha = -INF_SCORE, int beta = INF_SCORE);

        // checks if the time is up and throws an exception if it is
        void stop_check();

        // the elements exclusive to the worker
        Position pos, rootPos;

        HistoryEntry moveHistory[NUM_COLORS][HISTORY_SIZE];

        // shared elements
        TTable& tt;

        // only used by the master thread
        // horrendous but necessary to access the thread pool from the master thread
        ThreadPool<Worker>& threads;
        OutputManager& outputManager;
        SearchLimits limits;
        chr::time_point<chr::steady_clock> stopTime;
};


class SearchManager {
    public:
        SearchManager(size_t numThreads, OutputManager& outputManager) : 
            threads(numThreads, tt, threads, outputManager) {}

        void set_position(
            std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        );

        // functions to start and stop the search
        void start_searching() { threads.start_searching(); }
        void abort_search() { threads.abort_search(); }
        void wait_search_finished() { threads.wait_search_finished(); }

        bool is_searching() { return threads.is_searching(); }
        
        // function to get the results of the search
        SearchInfo get_results();

        void set_limits(const SearchLimits& limits) { threads.master().set_limits(limits); }

        void print_stats();

    private:
        Position pos;

        TTable tt;
        ThreadPool<Worker> threads;
};


} // namespace harukashogi

#endif // SEARCH_H