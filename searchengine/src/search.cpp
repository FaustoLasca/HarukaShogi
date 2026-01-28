#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>

#include "search.h"
#include "movegen.h"
#include "evaluate.h"
#include "misc.h"


namespace chr = std::chrono;

namespace harukashogi {


Move Searcher::search(chr::milliseconds timeLimit, int maxDepth) {
    iterative_deepening(timeLimit, maxDepth);
    return get_best_move();
}

std::string Searcher::search(int timeLimit, int maxDepth) {
    Move bestMove = search(chr::milliseconds(timeLimit), maxDepth);
    return bestMove.to_string();
}


int Searcher::iterative_deepening(chr::milliseconds timeLimit, int maxDepth) {
    // set the time limit and start time
    this->timeLimit = timeLimit;
    startTime = chr::steady_clock::now();
    timeUp = false;

    // initialize the required variables
    int score = 0;
    int depth;
    followingPV = false;
    bestMove = Move::null();
    // loop through the depths
    for (depth = 1; depth <= maxDepth; depth++) {
        try {
            score = min_max(depth);
        } catch (const TimeUpException& e) {
            // if the time is up, exit the loop
            break;
        }
        bestMove = pvTable[0];
        followingPV = true;
    }

    chr::milliseconds timeTaken = chr::duration_cast<chr::milliseconds>(chr::steady_clock::now() - startTime);
    int nodesPS = nodeCount / (timeTaken.count() / 1000.0);

    std::cout << "Evaluation: " << score << "\t Depth: " << depth - 1 << "\t Nodes: " << nodeCount
              << "\t Time: " << timeTaken.count() << "ms" << "\t Nodes/s: " << nodesPS << std::endl;

    return score;
}


int Searcher::min_max(int depth, int ply, int alpha, int beta) {
    // increment node count
    nodeCount++;

    // if the depth is 0, return the evaluation of the position
    if (depth == 0 || pos.is_game_over())
        return evaluate(pos);

    // if the time is up, throw an exception
    if (is_time_up())
        throw TimeUpException();

    // probe the transposition table for an entry
    std::tuple<bool, TTEntry*> result = tt.probe(pos.get_key());
    bool hit = std::get<0>(result);
    TTEntry* ttEntry = std::get<1>(result);
    // if an entry is found, check if the depth is equal or greater than the current depth
    // if it is, there are 3 possibilities to prune:
    // 1. PV_NODE  (exact score): return the score of the entry
    // 3. CUT_NODE (lower bound): return the score if it is greater than beta (pruned)
    // 2. ALL_NODE (upper bound): return the score if it is lower than alpha (already found better)
    if (hit) {
        if (ttEntry->depth >= depth) {
            // case 1: PV_NODE (exact score)
            if (ttEntry->nodeType == PV_NODE) {
                return ttEntry->score;
            }
            // case 2: CUT_NODE (lower bound)
            if (ttEntry->nodeType == CUT_NODE) {
                if (ttEntry->score >= beta) {
                    return ttEntry->score;
                }
            }
            // case 3: ALL_NODE (upper bound)
            if (ttEntry->nodeType == ALL_NODE) {
                if (ttEntry->score < alpha) {
                    return ttEntry->score;
                }
            }
        }
    }
    // initialize the node type to ALL_NODE
    // if the entry is pruned, set the node type to CUT_NODE
    // if a score >= alpha, set the node type to PV_NODE
    NodeType nodeType = ALL_NODE;

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
    Move bestMove = Move::null();
    int score;
    for (ValMove* m = scoredMoves; m < endScored; ++m) {

        pos.make_move(*m);
        // in case of time up, unmake the move before stopping
        // the pos must be restored to the original position
        try {
            score = -min_max(depth - 1, ply + 1, -beta, -alpha);
        } catch (const TimeUpException& e) {
            pos.unmake_move(*m);
            throw e;
        }
        
        pos.unmake_move(*m);

        // update best score and the pv table
        if (score > best_score) {
            best_score = score;
            bestMove = *m;

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
            if (score >= alpha) {
                alpha = score;
                nodeType = PV_NODE;

                // if alpha is greater than beta, prune the search
                // fail soft
                if (alpha >= beta) {
                    ttEntry->key = pos.get_key();
                    ttEntry->score = best_score;
                    ttEntry->depth = depth;
                    ttEntry->nodeType = CUT_NODE;
                    ttEntry->bestMove = bestMove;
                    return best_score;
                }
            }
        } 
    }

    ttEntry->key = pos.get_key();
    ttEntry->score = best_score;
    ttEntry->depth = depth;
    ttEntry->nodeType = nodeType;
    ttEntry->bestMove = bestMove;

    return best_score;
}


bool Searcher::is_time_up() {
    if (timeUp) return true;

    timeUp = (std::chrono::steady_clock::now() - startTime) > timeLimit;
    return timeUp;
}


void Searcher::set_position(std::string sfen) {
    pos = Position();
    pos.set(sfen);
    bestMove = Move::null();
    pvTable.fill(Move::null());
    pvLength.fill(0);
    nodeCount = 0;
}

} // namespace harukashogi