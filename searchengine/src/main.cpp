#include <iostream>
#include <random>
#include <iomanip>
#include <bitset>

#include "movegen.h"
#include "types.h"
#include "position.h"
#include "search.h"
#include "misc.h"
#include "perft.h"
#include "ttable.h"

using namespace harukashogi;


int main() {
    
    Position::init();

    Searcher searcher;
    searcher.set_position("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");

    Move bestMove = searcher.search(chr::milliseconds(600000), 4);

    std::cout << "Best move: " << bestMove << std::endl;

    searcher.print_stats();

    return 0;
}