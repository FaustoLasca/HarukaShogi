#include <iostream>
#include <random>
#include <iomanip>
#include <bitset>

#include "movegen.h"
#include "types.h"
#include "position.h"
#include "search.h"
#include "misc.h"
#include "perft.h"

using namespace harukashogi;


int main() {
    Position::init();

    Position pos;
    pos.set("1r4k1l/1P4gs1/4+Sp3/l1p4pp/PN1p3n1/1BP2P1GP/K3P4/SG+b6/r7L w GNNLPPPPsppp 112");

    std::cout << pos.is_game_over() << " - " << int(pos.get_winner()) << std::endl;

    std::printf("Key: %16lx\n", pos.get_key());

    Move move = Move(SILVER, SQ_96);

    pos.make_move(move);
    
    std::cout << pos.is_game_over() << " - " << int(pos.get_winner()) << std::endl;

    std::cout << pos.sfen() << std::endl;

    // Move move = Move(PAWN, SQ_12);

    // std::cout << move << std::endl;
    // std::cout << "from: " << move.from() << std::endl;
    // std::cout << "dropped: " << int(move.dropped()) << std::endl;
    // std::cout << "to: " << move.to() << std::endl;
    // std::cout << "promotion: " << move.is_promotion() << std::endl;
    // std::cout << "drop: " << move.is_drop() << std::endl;
    // std::printf("Raw: %016b\n", move.raw());

    // perft_test(pos, 2);

    // std::printf("Key: %16lx\n", pos.get_key());

    return 0;
}