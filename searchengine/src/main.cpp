#include <iostream>
#include <bitset>
#include <chrono>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"
#include "search.h"

using namespace harukashogi;


int main() {
    init();

    Position pos;
    pos.set("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");
    // pos.set();
    int depth = 4;

    // Searcher searcher(false);
    // searcher.set_position();

    auto start = std::chrono::high_resolution_clock::now();

    std::cout << perft(pos, depth) << std::endl;
    // perft_test(pos, depth);
    // searcher.search(std::chrono::milliseconds(100000000), depth);


    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    // std::cout << sld_attacks_bb(SQ_55, WHITE, LANCE, pos.all_pieces()) << std::endl;

    return 0;
}