#include <iostream>
#include <string>

#include "position.h"
#include "perft.h"

using namespace harukashogi;


int main() {
    Position pos;
    pos.set("8l/lpsg1b3/3k5/pP2PpPrp/1Np4n1/P5G2/3PGS2P/4K2P1/L6RL b GSNNPPsbpppp 113");
    std::string sfen = pos.sfen();

    perft_test(pos, 3);

    std::cout << (pos.sfen() == sfen) << std::endl;

    return 0;
}