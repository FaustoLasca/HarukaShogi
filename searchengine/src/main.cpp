#include <iostream>
#include <string>
#include <chrono>

#include "position.h"
#include "perft.h"

using namespace harukashogi;


int main() {
    Position pos;
    pos.set("l+R1g3nl/2n1k1gs1/3ppp1pp/1Pps2P2/2P3S2/P1B6/2gPPP2P/2B1KL+r2/7NL b SNPgppp 81");
    std::string sfen = pos.sfen();

    auto start_time = std::chrono::high_resolution_clock::now();

    perft(pos, 4);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;

    std::cout << (pos.sfen() == sfen) << std::endl;

    return 0;
}