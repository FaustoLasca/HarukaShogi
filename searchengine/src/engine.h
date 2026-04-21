#ifndef ENGINE_H
#define ENGINE_H

#include "search.h"
#include "opening_book.h"

namespace harukashogi {


void init();


class Engine {
    public:
        Engine(OutputManager& outputManager) : 
            outputManager(outputManager),
            threads(tt, threads, outputManager) 
        {
            init();
        }

        // options
        void resize_tt(size_t size) { tt.resize(size); }
        void resize_threadpool(size_t numThreads) { threads.resize(numThreads); }
        void set_move_overhead(int overhead) { threads.master().set_move_overhead(overhead); }
        void set_own_book(bool ownBook) { this->ownBook = ownBook; }

        void new_game();
        void set_position(const std::string& sfen, const std::vector<std::string>& moves = {});
        void go(const SearchLimits& limits);
        void stop();
        void ponderhit();

    private:
        Position pos;
        TTable tt;
        ThreadPool<Worker> threads;
        OutputManager& outputManager;
        OpeningBook openingBook;
        bool ownBook = true;
};


} // namespace harukashogi

#endif