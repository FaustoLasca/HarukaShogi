#include <iostream>
#include <string>

#include "position.h"
#include "types.h"
#include "movegen.h"

using namespace harukashogi;


int count_leaves(Position& pos, int depth) {
    if (depth == 0)
        return 1;

    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);

    // manually check game over
    if (moveList[0].is_null())
        return 1;

    int nodes = 0;
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        nodes += count_leaves(pos, depth - 1);
        pos.undo_move(*m);
    }

    return nodes;
};


void perft(Position& pos, int depth) {
    std::cout << "Perft(" << depth << ") - sfen: " << pos.sfen() << std::endl;

    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);

    int count = 0;
    std::string sfen;

    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        sfen = pos.sfen();
        count = count_leaves(pos, depth - 1);
        pos.undo_move(*m);

        std::cout << "perft: " << count << " - sfen: " << sfen << std::endl;
    }
}


int main() {
    Position pos;
    pos.set("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
    std::string sfen = pos.sfen();

    // for (int depth = 0; depth <= 6; ++depth)
    //     std::cout << count_leaves(pos, depth) << std::endl;
    perft(pos, 6);

    std::cout << (pos.sfen() == sfen) << std::endl;

    // std::cout << pos.sfen() << std::endl;

    // Move moveList[MAX_MOVES];
    // Move* end = generate_moves(pos, moveList);
    // for (Move* m = moveList; m < end; ++m) {
    //     std::cout << "Move: " << int(m->from) << " " << int(m->to) << " " << int(m->type_involved) << " " << m->promotion << std::endl;
    // }

    // std::cout << sfen << std::endl;
    // std::cout << pos.sfen() << std::endl;
    // pos.make_move(moveList[0]);
    // std::cout << pos.sfen() << std::endl;
    // pos.undo_move(moveList[0]);
    // std::cout << pos.sfen() << std::endl;

    return 0;
}