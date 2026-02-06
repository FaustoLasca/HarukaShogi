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
    pos.set("l3+B2Sl/1+r2k4/p2g2+Bpp/2p1ppN2/1l1pPP3/2Pn5/P1S5P/1P1G5/LKN5+r w GSPgsnpppp 124");
    int depth = 2;

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