#include <iostream>
#include <bitset>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"

using namespace harukashogi;


int main() {
    Position::init();

    Position pos;
    pos.set();

    Move moveList[MAX_MOVES];
    //Move* end = generate_all_direction<QUIET, BLACK>(pos, moveList);
    //for (Move* move = moveList; move < end; ++move)
    //    std::cout << *move << std::endl;

    Move*end = generate_sliding<BLACK, LANCE>(pos, moveList, ~pos.all_pieces(BLACK));
    for (Move* move = moveList; move < end; ++move)
        std::cout << *move << std::endl;

    return 0;
}