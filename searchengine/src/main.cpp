#include <iostream>
#include <bitset>
#include <chrono>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"

using namespace harukashogi;


int main() {
    init();

    Position pos;
    pos.set("ln1g5/1r3kg2/p2pppn+P1/2ps2p2/1p6l/2P6/PPSPPPPLN/2G2K1pP/LN4G1b w BSSPr 56");
    int depth = 1;

    // Move moveList[MAX_MOVES];
    // Move* moveEnd = generate<LEGAL>(pos, moveList);
    // for (Move* move = moveList; move < moveEnd; ++move)
    //     std::cout << *move << std::endl;
    // std::cout << "Number of moves: " << moveEnd - moveList << std::endl;
    

    auto start = std::chrono::high_resolution_clock::now();

    perft_test(pos, depth);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    return 0;
}