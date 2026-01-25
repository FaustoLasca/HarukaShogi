#include <iostream>
#include <string>
#include <chrono>

#include "position.h"
#include "perft.h"
#include "evaluate.h"
#include "misc.h"
#include "search.h"

using namespace harukashogi;


int main() {
    Position pos;
    pos.set("9/9/2R1g4/S5P2/4k1+l2/9/3+P2+P2/9/8K b - 1");
    std::string sfen = pos.sfen();

    Searcher searcher;

    auto start_time = std::chrono::high_resolution_clock::now();

    int score = searcher.min_max(pos, 5);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    std::cout << "Score: " << score << " (" << sfen << ")" << std::endl;
    std::cout << "Nodes searched: " << searcher.get_node_count() << " (" << searcher.get_node_count() / (duration.count() / 1000.0) << " nps)" << std::endl;
    std::cout << "Best move: " << searcher.get_best_move() << std::endl;

    std::cout << (pos.sfen() == sfen) << std::endl;

    return 0;
}