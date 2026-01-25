#include <iostream>
#include <string>
#include <chrono>

#include "position.h"
#include "perft.h"
#include "evaluate.h"
#include "misc.h"
#include "search.h"

using namespace harukashogi;


int main() {
    Searcher searcher;
    searcher.set_position("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");

    std::string bestMove = searcher.search(500);

    std::cout << "Best Move: " << bestMove << std::endl;

    return 0;
}