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
    pos.set("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");
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