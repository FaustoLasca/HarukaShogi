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
    pos.set();
    int depth = 5;

    auto start = std::chrono::high_resolution_clock::now();

    perft_test(pos, depth);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    return 0;
}