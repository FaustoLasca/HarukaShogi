#include <iostream>
#include <immintrin.h>
#include <bitset>
#include <chrono>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"
#include "search.h"
#include "movepicker.h"
#include "history.h"

using namespace harukashogi;


int main() {
    init();

    TTable tt;
    Worker worker(tt);

    worker.set_position("ln1g3nl/2s1k1gsp/3ppp1p1/p1p3p1P/1P7/PG1PR4/2NKPPP1L/6S2/L+pb2G1N1 w RBPPs 60");
    worker.timeLimit = chr::milliseconds(1000);

    worker.start_searching();

    std::cout << "Depth:      " << worker.depth << std::endl;
    std::cout << "Evaluation: " << worker.eval << std::endl;
    std::cout << "Best move:  " << worker.bestMove.to_string() << std::endl;
    std::cout << "Node count: " << worker.nodeCount << std::endl;
    std::cout << "TT Stats" << std::endl;
    tt.print_stats();

    return 0;
}