#include <iostream>

#include "nnue.h"
#include "position.h"
#include "evaluate.h"
#include "engine.h"
#include "movegen.h"


using namespace harukashogi;

int main() {
    init();

    Position pos;
    pos.set("ln3k1nl/1r2g1gs1/4pp1p1/p1pps1p1p/5P3/P1P1S1PRP/1PSPP4/2G1G4/LN1K3NL w BPbp 1");
    NNUE::NNUE nnue;
    NNUE::Accumulator acc;
    Move moveList[MAX_MOVES], *end, selectedMove;

    nnue.compute_accumulator(acc, pos);
    std::cout << "sfen: " << pos.sfen() << std::endl;
    std::cout << "acc: ";
    for (int i = 0; i < 8; ++i)
        std::cout << acc.v[0][i] << " ";
    for (int i = 0; i < 8; ++i)
        std::cout << acc.v[1][i] << " ";
    std::cout << std::endl;
    std::cout << "NNUE Score: " << nnue.evaluate(acc, pos.side_to_move()) << std::endl;
    std::cout << "EVAL Score: " << evaluate(pos) << std::endl;

    for (int i = 0; i < 20; ++i) {
        end = generate<LEGAL>(pos, moveList);
        selectedMove = moveList[rand() % (end - moveList)];    
        nnue.update_accumulator(acc, pos, selectedMove);
        pos.make_move(selectedMove);

        std::cout << "---------------------------------------------------------------" << std::endl;
        std::cout << "acc: ";
        for (int i = 0; i < 8; ++i)
            std::cout << acc.v[0][i] << " ";
        for (int i = 0; i < 8; ++i)
            std::cout << acc.v[1][i] << " ";
        std::cout << std::endl;
        std::cout << "sfen: " << pos.sfen() << std::endl;
        std::cout << "NNUE Score: " << nnue.evaluate(acc, pos.side_to_move()) << std::endl;
        std::cout << "EVAL Score: " << evaluate(pos) << std::endl;
    }
}