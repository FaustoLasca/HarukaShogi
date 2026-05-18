#include <iostream>
#include <fstream>
#include <bitset>

#include "position.h"
#include "nnue/binpack.h"
#include "misc.h"
#include "types.h"

using namespace harukashogi;


int main() {
    Position::init();

    Position pos;
    pos.set();

    NNUE::Binpack binpack("../HarukaShogi/data/test_binps/0.binp", std::ios::in);
    NNUE::GameData game;
    while (binpack.read_game(game)) {
        std::cout << "sfen:      " << game.pos.sfen() << std::endl;
        std::cout << "winner:    " << (int)game.winner << std::endl;
        std::cout << "movecount: " << game.scoreMoves.size() << std::endl;
        for (const auto& el : game.scoreMoves) {
            std::cout << "el: " << std::get<0>(el) << " " << std::get<1>(el) << " " << std::get<2>(el) << std::endl;
        }
    }
}