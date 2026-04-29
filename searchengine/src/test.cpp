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

    {
        NNUE::Binpack oBinpack("test.binp", std::ios::out);

        Move move = move_from_string("7g7f");
        int16_t score = 100;
        bool discard = false;

        oBinpack.new_game(pos);
        for (int i = 0; i < 10; ++i) {
            oBinpack.add_move(move, score, discard);
        }
        oBinpack.game_over(NO_COLOR);

        pos.set("ln4k1l/4g2s1/3s1pnp1/3pp1P1p/P1PP1P3/2p1S2RP/1+r2P4/L3+n4/1+p2K2NL b BGSPPbggpp 1");
        oBinpack.new_game(pos);
        for (int i = 0; i < 3; ++i) {
            oBinpack.add_move(move, -score, ~discard);
        }
        oBinpack.game_over(BLACK);
    }

    {
        NNUE::Binpack iBinpack("test.binp", std::ios::in);
        NNUE::GameData game;
        while (iBinpack.read_game(game)) {
            std::cout << "sfen:      " << game.pos.sfen() << std::endl;
            std::cout << "winner:    " << (int)game.winner << std::endl;
            std::cout << "movecount: " << game.scoreMoves.size() << std::endl;
            for (const auto& el : game.scoreMoves) {
                std::cout << "el: " << std::get<0>(el) << " " << std::get<1>(el) << " " << std::get<2>(el) << std::endl;
            }
        }
    }
}