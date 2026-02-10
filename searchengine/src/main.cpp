#include <iostream>
#include <immintrin.h>
#include <bitset>
#include <chrono>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"
#include "search.h"
#include "movepicker.h"

using namespace harukashogi;


int main() {
    init();

    Searcher searcher(false);
    searcher.set_position("1r4k1l/1P4gs1/4+Sp3/l1pB3pp/PN1p3n1/2P2P1GP/K2+bP4/SG7/r7L w GNNLPPPPsppp 110");

    std::cout << searcher.search(600000, 6) << std::endl;

    // Position pos;
    // pos.set("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");
    // MovePicker movePicker(pos, 6, Move(SQ_55, SQ_16));
    // Move m;
    // m = movePicker.next_move();
    // std::cout << m << std::endl;

    return 0;
}