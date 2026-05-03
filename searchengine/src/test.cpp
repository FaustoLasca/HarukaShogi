#include <iostream>

#include "position.h"
#include "nnue/binpack.h"
#include "nnue/nnue.h"
#include "misc.h"
#include "types.h"

using namespace harukashogi;


int main() {
    Position::init();

    Position pos;
    pos.set();

    NNUE::NNUE nnue;
    NNUE::AccumulatorType acc;

    NNUE::Binpack binpack("data/test_binps/0.binp", std::ios::in);
    NNUE::GameData game;

    binpack.read_game(game);
    size_t idx = 0;
    for (int i = 0; i < 10; i++) {
        nnue.feature_transformer().compute(game.pos, acc);
        std::cout << nnue.evaluate(acc, game.pos.side_to_move()) << std::endl;
        do {
            game.pos.make_move(std::get<0>(game.scoreMoves[idx]));
            idx++;
        } while(std::get<2>(game.scoreMoves[idx]));
    }
        
}