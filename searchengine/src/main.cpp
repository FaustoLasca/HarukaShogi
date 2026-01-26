#include <iostream>
#include <random>
#include <iomanip>
#include <bitset>

#include "movegen.h"
#include "types.h"
#include "position.h"
#include "search.h"
#include "misc.h"

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

        rt.count(pos.get_key());

        rt.remove(pos.get_key());
        pos.unmake_move(*m);
    }

    return nodes;
}


int main() {
    Position::init();

    Position pos;
    pos.set("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
    RepetitionTable rt;
    rt.add(pos.get_key());

    std::cout << "nodes: " << test_repetition_table(pos, rt, 6) << std::endl;
    std::cout << "hash hits: " << rt.get_counts_needed() << std::endl;
    std::cout << "repetitions: " << rt.get_repetitions() << std::endl;


    return 0;
}