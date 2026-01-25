#include <iostream>

#include "search.h"
#include "movegen.h"
#include "evaluate.h"

namespace harukashogi {


int Searcher::min_max(Position& pos, int depth, int ply, int alpha, int beta) {
    // increment node count
    nodeCount++;

    // if the depth is 0, return the evaluation of the position
    if (depth == 0 || pos.is_game_over())
        return evaluate(pos);

    // generate all moves from the position
    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);

    // check for stalemate
    // extremely rare but possible
    if (moveList[0].is_null())
        return 0;

    // loop through children nodes
    int best_score = -INF_SCORE;
    int score;
    for (Move* m = moveList; m < end; ++m) {
        pos.make_move(*m);
        score = -min_max(pos, depth - 1, -beta, -alpha);
        pos.unmake_move(*m);

        // update best score and move
        if (score > best_score) {
            best_score = score;
            if (ply == 0)
                bestMove = *m;

            // alpha-beta pruning
            // if the score is greater than alpha, update alpha
            if (score > alpha) {
                alpha = score;

                // if alpha is greater than beta, prune the search
                // fail soft
                if (alpha >= beta)
                    return best_score;
            }
        } 
    }

    return best_score;
}


} // namespace harukashogi