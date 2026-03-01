#ifndef ENGINE_H
#define ENGINE_H

#include "search.h"
#include "misc.h"

namespace harukashogi {


class USIManager : public OutputManager {
    public:
        void on_best_move(Move bestMove, Move ponderMove) override;
        void on_iter(const SearchInfo& info) override;
    
    private:
        std::atomic<bool> debug = false;
};


class USIEngine {
    public:
        USIEngine(int numThreads) : searchManager(numThreads, usiManager) {
            init();
        }

        void run();

    private:
        // usi commands
        void usi();
        void debug(std::istringstream& cmdStream);
        void isready();
        void setoption(std::istringstream& cmdStream);
        // void register();
        void usinewgame();
        void position(std::istringstream& cmdStream);
        void go(std::istringstream& cmdStream);
        void stop();
        void ponderhit();
        void gameover(std::istringstream& cmdStream);
        // void quit();

        USIManager usiManager;
        SearchManager searchManager;
};


} // namespace harukashogi

#endif // ENGINE_H