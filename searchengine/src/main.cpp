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
    searcher.set_position("1r4k1l/1P4gs1/4+Sp3/l1pB3pp/PN1p3n1/2P2P1GP/K2+bP4/SG7/r7L w GNNLPPPPsppp 110");

    Move bestMove = searcher.search(chr::milliseconds(600000), 5);

    std::cout << "Best move: " << bestMove << std::endl;

    searcher.tt.print_stats();

    return 0;
}