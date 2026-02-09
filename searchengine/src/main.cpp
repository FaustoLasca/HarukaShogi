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
    pos.set();

    constexpr Color c = BLACK;
    constexpr PieceType pt = BISHOP;
    constexpr Piece p = make_piece(c, pt);
    constexpr Square sq = SQ_55;

    std::cout << sld_attacks_bb(sl_dir_index(p), sq, pos.all_pieces()) << std::endl;

    return 0;
}