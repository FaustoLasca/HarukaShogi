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
    Move* end = generate_direction_moves<QUIET, BLACK>(pos, moveList);
    for (Move* move = moveList; move < end; ++move)
        std::cout << *move << std::endl;

    return 0;
}