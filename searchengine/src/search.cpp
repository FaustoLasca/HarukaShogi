#include "search.h"
#include "movegen.h"
#include "evaluate.h"

namespace harukashogi {


int Searcher::min_max(Position& pos, int depth) {
    // if the depth is 0, return the evaluation of the position
    if (depth == 0 || pos.is_game_over())
        return evaluate(pos);

    // generate all moves from the position
    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);

    if (moveList[0].is_null())
        return 0;

    int best_score = -INF_SCORE;
    int score;
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        score = -min_max(pos, depth - 1);
        pos.unmake_move(*m);

        if (score > best_score)
            best_score = score;
    }

    return best_score;
}


} // namespace harukashogi