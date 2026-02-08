#include <iostream>
#include <bitset>
#include <chrono>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"
#include "search.h"

using namespace harukashogi;


int main() {
    init();

    Position pos;
    pos.set("4k2nl/2g1g1+P2/+L3p3p/2ps1rNb1/4NpP2/pS1PP4/1PP1SP2P/K6+b1/3G3NL b GSLPPPPrp 85");
    // pos.set();
    int depth = 2;

    // Searcher searcher(false);
    // searcher.set_position();

    auto start = std::chrono::high_resolution_clock::now();

    // std::cout << perft(pos, depth) << std::endl;
    perft_test(pos, depth);
    // searcher.search(std::chrono::milliseconds(100000000), depth);


    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    // std::cout << pos.all_pieces() << std::endl;
    // std::cout << pos.all_pieces(BLACK) << std::endl;
    // std::cout << pos.all_pieces(WHITE) << std::endl;
    // std::cout << pos.pieces(WHITE, PAWN) << std::endl;
    // std::cout << pos.pieces(WHITE, GOLD) << std::endl;
    // std::cout << pos.pieces(WHITE, BISHOP) << std::endl;
    // std::cout << pos.pieces(WHITE, ROOK) << std::endl;
    // std::cout << pos.pieces(WHITE, SILVER) << std::endl;
    // std::cout << pos.pieces(WHITE, LANCE) << std::endl;
    // std::cout << pos.pieces(WHITE, KNIGHT) << std::endl;
    // std::cout << pos.pieces(WHITE, P_BISHOP) << std::endl;
    // std::cout << pos.pieces(WHITE, P_ROOK) << std::endl;

    return 0;
}