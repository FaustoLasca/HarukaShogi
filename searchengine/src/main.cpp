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

        // rt.reached_repetitions(pos.get_key(), pos.si, 2);

        rt.remove(pos.get_key());
        pos.unmake_move(*m);
    }

    return nodes;
}


int main() {
    Position::init();

    Position pos;
    pos.set();

    std::printf("Key: %16lx\n", pos.get_key());

    perft_test(pos, 5);

    std::printf("Key: %16lx\n", pos.get_key());

    return 0;
}