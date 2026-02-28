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
        Worker(size_t id, TTable& tt) : Thread(id), tt(tt), threadId(id) {}

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
        int q_search(int alpha = -INF_SCORE, int beta = INF_SCORE);

        // the elements exclusive to the worker
        Position pos;

        HistoryEntry moveHistory[NUM_COLORS][HISTORY_SIZE];

        // shared elements
        TTable& tt;

        size_t threadId;
};


class SearchManager {
    public:
        SearchManager(size_t numThreads) : threads(numThreads, tt) {}

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

        void print_stats();

    private:
        Position pos;

        TTable tt;
        ThreadPool<Worker> threads;
};


// TODO: TEMPORARY SEARCHER CLASS
//       HAS TO BE REMOVED/REFACTORED AFTER IMPLEMENTING PARALLEL SEARCH
class Searcher {
    public:
        Searcher(bool useOpeningBook = false) : useOpeningBook(useOpeningBook), searchManager(8) {}

        void set_position(
            std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        );

        Move search(chr::milliseconds timeLimit, int depth);
        std::string search(int timeLimit, int depth);

        void print_stats();

    private:
        Position pos;
        bool useOpeningBook;
        OpeningBook openingBook;

        SearchManager searchManager;
};


} // namespace harukashogi

#endif // SEARCH_H