#include <iostream>


#include "nnue/nnue.h"
#include "engine.h"


using namespace harukashogi;

int main() {
    init();

    Position pos;
    NNUE::NNUE nnue;

    pos.set("ln4k1l/4g2s1/3s1pnp1/3pp1P1p/P1PP1P3/2p1S2RP/1+r2P4/L3+n4/1+p2K2NL w BGSPPbggpp 1");
    NNUE::Accumulator acc;
    nnue.compute_accumulator(acc, pos);
    std::cout << nnue.evaluate(acc, pos.side_to_move()) << std::endl;
}