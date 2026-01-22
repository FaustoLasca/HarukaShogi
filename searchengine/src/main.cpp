#include <iostream>
#include <string>

#include "position.h"
#include "types.h"


using namespace harukashogi;

int main() {
    Position pos;
    std::string sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
    std::cout << "SFEN: " << sfen << std::endl;

    pos.set(sfen);
    std::cout << "SFEN: " << pos.sfen() << std::endl;

    Move m = { SQ_17, SQ_13, true, PAWN };
    pos.make_move(m);
    std::cout << "SFEN: " << pos.sfen() << std::endl;

    pos.undo_move(m);
    std::cout << "SFEN: " << pos.sfen() << std::endl;

    return 0;
}