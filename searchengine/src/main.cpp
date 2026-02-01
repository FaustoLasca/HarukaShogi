#include <iostream>
#include <bitset>

#include "bitboard.h"
#include "misc.h"

using namespace harukashogi;


int main() {
    
    Bitboard bb = square_bb(SQ_94) |
                  square_bb(SQ_35) |
                  square_bb(SQ_19);
    std::cout << bb << popcount(bb) << std::endl;

    Bitboard attacks = dir_attacks_bb<SE_DIR>(bb);
    std::cout << attacks << popcount(attacks) << std::endl;

    return 0;
}