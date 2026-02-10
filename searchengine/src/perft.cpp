#include <iostream>

#include "perft.h"
#include "movegen.h"
#include "types.h"
#include "misc.h"

namespace harukashogi {


int perft(Position& pos, int depth) {
    if (depth == 0 || pos.is_game_over()) {
        return 1;
    }

    Move moveList[MAX_MOVES];
    Move* end = generate<LEGAL>(pos, moveList);

    // in case of stalemate
    // extremely rare but possible
    if (end == moveList) {
        return 1;
    }

    int count = 0;
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        count += perft(pos, depth - 1);

        pos.unmake_move(*m);
    }

    return count;
}


int perft(std::string sfen, int depth) {
    Position pos;
    pos.set(sfen);
    return perft(pos, depth);
}


void perft_test(Position& pos, int depth) {
    std::cout << "Perft test at depth " << depth << std::endl;
    std::cout << "sfen: " << pos.sfen() << std::endl;
    std::cout << std::endl;

    int count;

    for (int i = 0; i <= depth; ++i) {
        count = perft(pos, i);
        std::cout << "Depth " << i << "\t -  " << count << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Perft test for each move" << std::endl;
    Move moveList[MAX_MOVES];
    Move* end = generate<LEGAL>(pos, moveList);
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        count = perft(pos, depth - 1);
        std::cout << *m << "\t -  " << count << " \t -  " << pos.sfen() << std::endl;
        pos.unmake_move(*m);
    }
}

void perft_test(std::string sfen, int depth) {
    Position pos;
    pos.set(sfen);
    perft_test(pos, depth);
}

} // namespace harukashogi