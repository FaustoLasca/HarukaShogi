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


int movepick_test(Position& pos, int depth) {
    if (depth == 0 || pos.is_game_over()) {
        return 1;
    }

    MovePicker movePicker(pos, depth, Move::null());

    int count = 0;
    Move m;
    while ((m = movePicker.next_move()) != Move::null()) {
        pos.make_move(m);
        count+= movepick_test(pos, depth - 1);
        pos.unmake_move(m);
    }

    if (count == 0) {
        return 1;
    }

    return count;
}


int main() {
    init();

    Searcher searcher(false);
    searcher.set_position();

    std::cout << searcher.search(600000, 6) << std::endl;

    // Position pos;
    // pos.set("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");
    // MovePicker movePicker(pos, 6, Move(SQ_55, SQ_16));
    // Move m;
    // m = movePicker.next_move();
    // std::cout << m << std::endl;

    // Position pos;
    // pos.set("1r4k1l/1P4gs1/4+Sp3/l1p4pp/PN1p3n1/1BP2P1GP/K3P4/SG+b6/r7L w GNNLPPPPsppp 112");
    // std::cout << perft(pos, 3) << std::endl;
    // std::cout << movepick_test(pos, 3) << std::endl;

    return 0;
}