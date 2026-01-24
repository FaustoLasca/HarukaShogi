#include <iostream>
#include <string>

#include "position.h"
#include "perft.h"

using namespace harukashogi;


int main() {
    Position pos;
    pos.set("1n5bl/lr2gkgP1/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL b Bp 51");
    std::string sfen = pos.sfen();

    perft_test(pos, 1);

    std::cout << (pos.sfen() == sfen) << std::endl;

    return 0;
}