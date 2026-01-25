#include <iostream>
#include <string>
#include <chrono>

#include "position.h"
#include "perft.h"
#include "evaluate.h"
#include "search.h"

using namespace harukashogi;


int main() {
    Position pos;
    pos.set("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");
    std::string sfen = pos.sfen();

    Searcher searcher;

    auto start_time = std::chrono::high_resolution_clock::now();

    int score = searcher.min_max(pos, 4);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    std::cout << "Score: " << score << " (" << sfen << ")" << std::endl;

    std::cout << (pos.sfen() == sfen) << std::endl;

    return 0;
}