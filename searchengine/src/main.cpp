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

    Searcher searcher(false);

    searcher.set_position();
    std::cout << searcher.search(600000000, 5) << std::endl;
    searcher.print_stats();

    searcher.set_position("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");
    std::cout << searcher.search(600000000, 5) << std::endl;
    searcher.print_stats();

    searcher.set_position("ln1g5/1r3kg2/p2pppn+P1/2ps2p2/1p6l/2P6/PPSPPPPLN/2G2K1pP/LN4G1b w BSSPr 56");
    std::cout << searcher.search(600000000, 5) << std::endl;
    searcher.print_stats();

    searcher.set_position("8l/lpsg1b3/3k5/pP2PpPrp/1Np4n1/P5G2/3PGS2P/4K2P1/L6RL b GSNNPPsbpppp 113");
    std::cout << searcher.search(600000000, 5) << std::endl;
    searcher.print_stats();

    searcher.set_position("ln1g3nl/2s1k1gsp/3ppp1p1/p1p3p1P/1P7/PG1PR4/2NKPPP1L/6S2/L+pb2G1N1 w RBPPs 60");
    std::cout << searcher.search(600000000, 5) << std::endl;
    searcher.print_stats();

    return 0;
}