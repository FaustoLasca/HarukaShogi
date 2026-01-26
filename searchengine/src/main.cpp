#include <iostream>
#include <random>
#include <iomanip>

#include "movegen.h"
#include "types.h"
#include "position.h"
#include "search.h"
#include "misc.h"

using namespace harukashogi;


int main() {
    Position::init();
    Searcher searcher;
    searcher.set_position("8l/lpsg1b3/3k5/pP2PpPrp/1Np4n1/P5G2/3PGS2P/4K2P1/L6RL b GSNNPPsbpppp 113");

    std::string bestMove = searcher.search(1000, 6);

    std::cout << "Best move: " << bestMove << std::endl;

    return 0;
}