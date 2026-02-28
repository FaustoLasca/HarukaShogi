#ifndef ENGINE_H
#define ENGINE_H

#include <queue>
#include <string>
#include <string_view>
#include <vector>

#include "types.h"
#include "search.h"

namespace harukashogi {


class USIEngine {
    public:
        USIEngine(int numThreads) : searchManager(numThreads) {}

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

        SearchManager searchManager;
};


} // namespace harukashogi

#endif // ENGINE_H