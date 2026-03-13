#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <exception>

#include "position.h"
#include "evaluate.h"
#include "ttable.h"
#include "history.h"
#include "thread.h"
#include "types.h"

namespace chr = std::chrono;

namespace harukashogi {


class AbortSearchException : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Time limit exceeded";
        }
};


constexpr int MAX_DEPTH = 20;


struct SearchInfo {
    std::array<Move, MAX_DEPTH> pv;
    int eval = 0;
    int depth = 0;
    uint64_t nodeCount = 0;
    chr::time_point<chr::steady_clock> startTime = chr::steady_clock::now();
};


struct StackEntry {
    std::array<Move, MAX_DEPTH> pv;
    int ply;
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
    uint64_t nodes;
    int depth, movesToGo;
    bool infinite, ponder;
};


class OutputManager {
    public:
        virtual void on_best_move(Move bestMove, Move ponderMove) = 0;
        virtual void on_iter(const SearchInfo& info) = 0;
};


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

        // master thread only
        void set_limits(const SearchLimits& limits) {
            // only the master thread uses the limits
            assert(is_master());
            this->limits = limits;
        };
        void set_stop(bool stop) {
            assert(is_master());
            this->stop = stop;
        }
        void set_ponderhit(bool ponderhit) {
            assert(is_master());
            this->ponderhit = ponderhit;
        };

        // struct containing the results and stats of the search
        SearchInfo info;
    private:

        void search() override;

        const Worker& get_best_thread();

        // iteratively performs searches at increasing depths
        void iterative_deepening();
        // the main search function
        template <bool isRoot>
        int search(StackEntry* stack, int depth, int alpha = -INF_SCORE, int beta = INF_SCORE);
        // quiescence search, called by the main search
        int q_search(int alpha = -INF_SCORE, int beta = INF_SCORE);

        // checks if the time is up and throws an exception if it is
        void stop_check();

        // the elements exclusive to the worker
        Position searchPos, rootPos;

        HistoryEntry moveHistory[NUM_COLORS][HISTORY_SIZE];

        StackEntry stack[MAX_DEPTH];
        void empty_stack();

        // shared elements
        TTable& tt;

        // only used by the master thread
        // horrendous but necessary to access the thread pool from the master thread
        ThreadPool<Worker>& threads;
        OutputManager& outputManager;
        SearchLimits limits;
        std::atomic<bool> stop = false;
        std::atomic<bool> ponderhit = false;
        chr::time_point<chr::steady_clock> stopTime;
};


} // namespace harukashogi

#endif // SEARCH_H