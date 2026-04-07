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
    pos.set("lr3g1nl/3sg1kb1/p1np1s1p1/2p1ppp1p/2P6/1P1P1P2P/Pp1SP1PP1/2R1G1SK1/LN2BG1NL w - 1");
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