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
        TestThread(int id) : Thread(id) {}

    protected:
        void search() override {
            std::cout << "Searching..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Search finished" << std::endl;
        }
};


int main() {
    init();

    // TTable tt;
    // Worker worker(tt);

    // worker.set_position("ln1g3nl/2s1k1gsp/3ppp1p1/p1p3p1P/1P7/PG1PR4/2NKPPP1L/6S2/L+pb2G1N1 w RBPPs 60");
    // worker.timeLimit = chr::milliseconds(1000);

    // worker.start_searching();

    // std::cout << "Depth:      " << worker.depth << std::endl;
    // std::cout << "Evaluation: " << worker.eval << std::endl;
    // std::cout << "Best move:  " << worker.bestMove.to_string() << std::endl;
    // std::cout << "Node count: " << worker.nodeCount << std::endl;
    // std::cout << "TT Stats" << std::endl;
    // tt.print_stats();

    TestThread thread(0);
    std::cout << "Thread created" << std::endl;

    thread.start_searching();
    std::cout << "Thread started" << std::endl;

    thread.wait_search_finished();
    std::cout << "Thread finished" << std::endl;

    thread.exit();
    std::cout << "Thread exited" << std::endl;

    return 0;
}