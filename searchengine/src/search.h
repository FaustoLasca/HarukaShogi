#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "evaluate.h"

namespace harukashogi {


class Searcher {
    public:
        Searcher() = default;

        int min_max(Position& pos, int depth);
};


} // namespace harukashogi

#endif // SEARCH_H