#ifndef ENGINE_H
#define ENGINE_H

#include "search.h"
#include "misc.h"

namespace harukashogi {


class Engine {
    public:
        Engine(OutputManager& outputManager) : threads(tt, threads, outputManager) {
            init();
        }

        void resize_tt(size_t size) { tt.resize(size); }
        void resize_threadpool(size_t numThreads) { threads.resize(numThreads); }

        // void new_game();
        void set_position(const std::string& sfen, const std::vector<std::string>& moves);
        void go(const SearchLimits& limits);
        void stop();
        void ponderhit();

    private:
        Position pos;
        TTable tt;
        ThreadPool<Worker> threads;
};


} // namespace harukashogi

#endif