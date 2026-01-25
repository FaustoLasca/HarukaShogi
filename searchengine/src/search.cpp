#include <iostream>
#include <algorithm>

#include "search.h"
#include "movegen.h"
#include "evaluate.h"
#include "misc.h"

namespace harukashogi {


int Searcher::iterative_deepening(Position& pos, int depth) {
    int score = 0;
    for (int d = 1; d <= depth; d++) {
        score = min_max(pos, d);
        followingPV = true;
    }
    return score;
}


int Searcher::min_max(Position& pos, int depth, int ply, int alpha, int beta) {
    // increment node count
    nodeCount++;

    // if the depth is 0, return the evaluation of the position
    if (depth == 0 || pos.is_game_over())
        return evaluate(pos);

    // generate all moves from the position
    Move moveList[MAX_MOVES];
    Move* end = generate_moves(pos, moveList);

    // if the ply is greater than the length of the pv, stop following the pv
    if (ply >= pvLength[0])
        followingPV = false;

    // evaluate all moves and sort them by value in descending order
    ValMove scoredMoves[MAX_MOVES];
    ValMove* endScored = scoredMoves;
    for (Move* m = moveList; m < end; ++m) {
        *endScored = *m;
        // if we are following the pv
        // evaluate the pv move with the max value
        if (followingPV && *m == pvTable[ply])
            endScored->value = INF_SCORE;
        else
            endScored->value = evaluate_move(pos, *m);
        endScored++;
    }

    std::sort(scoredMoves, endScored, [](const ValMove& a, const ValMove& b) {
        return a.value > b.value;
    });

    // check for stalemate
    // extremely rare but possible
    if (moveList[0].is_null())
        return 0;

    // loop through children nodes
    int best_score = -INF_SCORE;
    int score;
    for (ValMove* m = scoredMoves; m < endScored; ++m) {
        pos.make_move(*m);
        score = -min_max(pos, depth - 1, ply + 1, -beta, -alpha);
        pos.unmake_move(*m);

        // update best score and the pv table
        if (score > best_score) {
            best_score = score;
            
            // update the pv table for the current ply
            // the pv starts with the best move for the current ply
            // then continues with the pv of the next ply
            pvTable[ply * MAX_DEPTH] = *m;
            pvLength[ply] = 1 + pvLength[ply + 1];
            for (int i = 0; i < pvLength[ply + 1]; i++) {
                pvTable[ply * MAX_DEPTH + i + 1] = pvTable[(ply + 1) * MAX_DEPTH + i];
            }

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