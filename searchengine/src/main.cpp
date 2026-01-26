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


int test_repetition_table(Position& pos, RepetitionTable& rt, int depth) {
    if (depth == 0 || pos.is_game_over()) {
        return 1;
    }

    int nodes = 0;

    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);

    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        rt.add(pos.get_key());

        nodes += test_repetition_table(pos, rt, depth - 1);

        rt.reached_repetitions(pos.get_key(), 2);

        rt.remove(pos.get_key());
        pos.unmake_move(*m);
    }

    return nodes;
}


int main() {
    Position::init();

    Position pos;
    pos.set("l+R1g3nl/2n1k1gs1/3ppp1pp/1Pps2P2/2P3S2/P1B6/2gPPP2P/2B1KL+r2/7NL b SNPgppp 81");

    perft_test(pos, 4);

    return 0;
}