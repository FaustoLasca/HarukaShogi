#include <iostream>
#include <immintrin.h>
#include <bitset>
#include <chrono>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"
#include "search.h"
#include "movepicker.h"
#include "history.h"
#include "thread.h"

using namespace harukashogi;


class TestThread : public Thread {
    public:
        TestThread(size_t id, int a, int b) : Thread(id), a(a), b(b) {}

    private:
        void search() override {
            std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
            std::cout << "Thread " << threadId << " is searching" << std::endl;
            std::cout << "a: " << a << ", b: " << b << std::endl;
        }

        int a, b;
};


int main() {
    init();

    // Searcher searcher;
    // searcher.set_position();

    // searcher.search(1000, 20);

    // searcher.print_stats();

    // SearchManager searchManager(8);

    // searchManager.set_position("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");

    // searchManager.start_searching();
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // searchManager.abort_search();
    // searchManager.wait_search_finished();

    // SearchInfo results = searchManager.get_results();
    // std::cout << "Results:    " << results.bestMove << std::endl;
    // std::cout << "Evaluation: " << results.eval << std::endl;
    // std::cout << "Depth:      " << results.depth << std::endl;
    // std::cout << "Node count: " << results.nodeCount << std::endl;
    // searchManager.print_stats();

    Searcher searcher(true);
    searcher.set_position();
    std::cout << searcher.search(1000, 20) << std::endl;
    searcher.print_stats();

    return 0;
}