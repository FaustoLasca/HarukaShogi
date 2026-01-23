#include <iostream>
#include <string>

#include "position.h"
#include "types.h"
#include "movegen.h"

using namespace harukashogi;


int perft(Position& pos, int depth) {
    if (depth == 0)
        return 1;

    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);
    int nodes = 0;
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        nodes += perft(pos, depth - 1);
        pos.undo_move(*m);
    }

    return nodes;
};


int main() {
    Position pos;
    std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
    pos.set(sfen);

    for (int depth = 0; depth <= 5; ++depth) {
        std::cout << "Perft(" << depth << "): " << perft(pos, depth) << std::endl;
    }

    return 0;
}