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
    searcher.set_position("1r4k1l/1P4gs1/4+Sp3/l1pB3pp/PN1p3n1/2P2P1GP/K2+bP4/SG7/r7L w GNNLPPPPsppp 110");

    std::string bestMove = searcher.search(1000);

    std::cout << "Best Move: " << bestMove << std::endl;

    return 0;
}