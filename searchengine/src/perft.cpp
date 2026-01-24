#include <iostream>
#include <chrono>

#include "perft.h"
#include "movegen.h"
#include "types.h"
#include "misc.h"

namespace harukashogi {


int perft(Position& pos, int depth) {
    if (depth == 0) {
        return 1;
    }

    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);

    if (moveList[0].is_null()) {
        return 1;
    }

    int count = 0;
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        count += perft(pos, depth - 1);
        pos.undo_move(*m);
    }

    return count;
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
    Move* end = generate_moves(pos, moveList);
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        count = perft(pos, depth - 1);
        pos.undo_move(*m);

        std::cout << *m << "\t -  " << count << std::endl;
    }
}

} // namespace harukashogi