#include <iostream>
#include <string>

#include "position.h"
#include "types.h"
#include "movegen.h"

using namespace harukashogi;

int main() {
    Position pos;
    std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";

    pos.set(sfen);
    std::cout << "SFEN: " << pos.sfen() << std::endl;

    Move m = { SQ_17, SQ_13, true, PAWN };
    pos.make_move(m);
    std::cout << "SFEN: " << pos.sfen() << std::endl;

    pos.undo_move(m);
    std::cout << "SFEN: " << pos.sfen() << std::endl;


    Move moveList[MAX_MOVES];
    Move* end = piece_moves(pos, moveList, SQ_59);
    for (Move* m = moveList; m < end; ++m) {
        std::cout << "Move: " << int(m->from) << " -> " << int(m->to) << " (Promotion: " << m->promotion << ", Captured: " << int(m->type_involved) << ")" << std::endl;
    }

    return 0;
}