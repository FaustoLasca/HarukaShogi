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

    Searcher searcher;
    searcher.set_position();

    searcher.search(1000, 20);

    searcher.print_stats();

    ThreadPool<TestThread> pool(3, 1, 2);
    pool.start_searching();
    pool.wait_search_finished();

    return 0;
}