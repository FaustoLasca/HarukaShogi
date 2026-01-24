#include <iostream>
#include <string>
#include <chrono>

#include "position.h"
#include "types.h"
#include "misc.h"
#include "perft.h"

using namespace harukashogi;


int main() {
    Position pos;
    pos.set("ln5bl/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL b BPp 49");
    std::string sfen = pos.sfen();

    perft_test(pos, 3);

    std::cout << (pos.sfen() == sfen) << std::endl;

    return 0;
}