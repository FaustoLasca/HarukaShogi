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
#include "types.h"

using namespace harukashogi;


int main() {
    init();

    Position pos;
    pos.set("ln6l/1r2gkg2/4psnp1/p1pps1p1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL w BPbp 48");

    constexpr Color c = BLACK;
    constexpr PieceType pt = BISHOP;
    constexpr Piece p = make_piece(c, pt);
    constexpr Square sq = SQ_55;

    std::cout << sld_attacks_bb(sl_dir_index(p), sq, pos.all_pieces()) << std::endl;


    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Perft: " << perft(pos, 4) << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;

    return 0;
}