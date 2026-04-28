#include <iostream>
#include <fstream>
#include <bitset>

#include "position.h"


using namespace harukashogi;

int main() {
    Position::init();

    unsigned char bytes[45];

    Position pos;
    pos.set("ln4k1l/4g2s1/3s1pnp1/3pp1P1p/P1PP1P3/2p1S2RP/1+r2P4/L3+n4/1+p2K2NL b BGSPPbggpp 1");

    pos.to_bytes(bytes);

    for (int i = 0; i < 45; ++i) {
        std::cout << i << ":\t" << std::bitset<8>(bytes[i]) << std::endl;
    }

    pos.from_bytes(bytes);

    std::cout << pos.sfen() << std::endl;
}