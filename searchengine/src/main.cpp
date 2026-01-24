#include <iostream>
#include <string>

#include "position.h"
#include "perft.h"

using namespace harukashogi;


int main() {
    Position pos;
    pos.set("1n1g5/lr3kg2/p2pppn+P1/2ps2p1B/1p7/2P5l/PPSPPPPLN/2G2K1pP/LN4G1b b SSPr 59");
    std::string sfen = pos.sfen();

    perft_test(pos, 1);

    std::cout << (pos.sfen() == sfen) << std::endl;

    return 0;
}