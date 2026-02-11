#include <iostream>
#include <algorithm>
#include <chrono>

#include "search.h"
#include "movegen.h"
#include "evaluate.h"
#include "movepicker.h"
#include "misc.h"


namespace chr = std::chrono;

namespace harukashogi {


Move Searcher::search(chr::milliseconds timeLimit, int maxDepth) {
    if (useOpeningBook) {
        Move bookMove = openingBook.sample_move(pos.get_key());
        if (!bookMove.is_null()) {
            std::cout << "Using opening book move: " << bookMove.to_string() << std::endl;
            return bookMove;
        }
    }
        

    iterative_deepening(timeLimit, maxDepth);
    Move move = get_best_move();
    return move;
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
    // loop through the depths
    for (depth = 1; depth <= maxDepth; depth++) {
        try {
            score = min_max(depth, 0);
        } catch (const TimeUpException& e) {
            // if the time is up, exit the loop
            break;
        }
    }

    chr::milliseconds timeTaken = chr::duration_cast<chr::milliseconds>(
        chr::steady_clock::now() - startTime
    );
    int nodesPS = nodeCount / (timeTaken.count() / 1000.0);

    std::cout << "Evaluation: " << score << "\t Depth: " << depth - 1 << "\t Nodes: " << nodeCount
              << "\t Time: " << timeTaken.count() << "ms" << "\t Nodes/s: " << nodesPS << std::endl;

    return score;
}


int Searcher::min_max(int depth, int ply, int alpha, int beta) {
    // if the depth is 0, return the evaluation of the position
    if (pos.is_game_over()) {
        nodeCount++;
        return evaluate(pos);
    }
    else if (depth == 0)
        return quiescence(alpha, beta);
        

    // if the time is up, throw an exception
    if (is_time_up())
        throw TimeUpException();

    nodeCount++;

    // probe the transposition table for an entry
    std::tuple<bool, TTEntry*> result = tt.probe(pos.get_key());
    bool ttHit = std::get<0>(result);
    TTEntry* ttEntry = std::get<1>(result);
    // if an entry is found, check if the depth is equal or greater than the current depth
    // if it is, there are 3 possibilities to prune:
    // 1. PV_NODE  (exact score): return the score of the entry
    // 3. CUT_NODE (lower bound): return the score if it is greater than beta (pruned)
    // 2. ALL_NODE (upper bound): return the score if it is lower than alpha (already found better)
    Move ttMove = Move::null();
    if (ttHit) {
        if (ttEntry->depth >= depth) {
            // case 1: PV_NODE (exact score)
            // if the ply is 0 we risk making an illegal move with a type 1 collision
            if (ttEntry->nodeType == PV_NODE && ply != 0) {
                return ttEntry->score;
            }
            // case 2: CUT_NODE (lower bound)
            if (ttEntry->nodeType == CUT_NODE && ttEntry->score >= beta)
                return ttEntry->score;
            // case 3: ALL_NODE (upper bound)
            if (ttEntry->nodeType == ALL_NODE && ttEntry->score < alpha)
                return ttEntry->score;
        }

        ttMove = ttEntry->bestMove;
    }
    // initialize the node type to ALL_NODE
    // if the entry is pruned, set the node type to CUT_NODE
    // if a score >= alpha, set the node type to PV_NODE
    NodeType nodeType = ALL_NODE;

    // initialize the move picker
    MovePicker movePicker(pos, depth, ttMove);

    // loop through children nodes
    int bestScore = -INF_SCORE;
    Move nodeBestMove = Move::null();
    int score;
    Move m;
    while ((m = movePicker.next_move()) != Move::null()) {

        pos.make_move(m);
        // in case of time up, unmake the move before stopping
        // the pos must be restored to the original position
        try {
            score = -min_max(depth - 1, ply + 1, -beta, -alpha);
        } catch (const TimeUpException& e) {
            pos.unmake_move(m);
            throw e;
        }
        
        pos.unmake_move(m);

        // update best score and the pv table
        if (score > bestScore) {
            bestScore = score;
            nodeBestMove = m;

            // alpha-beta pruning
            // if the score is greater than alpha, update alpha
            if (score >= alpha) {
                alpha = score;
                nodeType = PV_NODE;

                // if alpha is greater than beta, prune the search
                // fail soft
                if (alpha >= beta) {
                    ttEntry->key = pos.get_key();
                    ttEntry->score = bestScore;
                    ttEntry->depth = depth;
                    ttEntry->nodeType = CUT_NODE;
                    ttEntry->bestMove = nodeBestMove;
                    return bestScore;
                }
            }
        } 
    }

    ttEntry->key = pos.get_key();
    ttEntry->score = bestScore;
    ttEntry->depth = depth;
    ttEntry->nodeType = nodeType;
    ttEntry->bestMove = nodeBestMove;

    if (ply == 0)
        bestMove = nodeBestMove;

    return bestScore;
}


int Searcher::quiescence(int alpha, int beta) {
    nodeCount++;

    // if the time is up, throw an exception
    if (is_time_up())
        throw TimeUpException();

    int eval = evaluate(pos);

    if (eval >= beta)
        return eval;
    if (pos.is_game_over())
        return eval;

    int bestScore = eval;
    if (eval > alpha)
        alpha = eval;

    // initialize the move picker
    MovePicker movePicker(pos, 0);

    // search through the scored captures
    int score;
    Move m;
    while ((m = movePicker.next_move()) != Move::null()) {
        // only consider captures
        if (pos.is_capture(m) && pos.is_legal(m)) {

            pos.make_move(m);
            score = -quiescence(-beta, -alpha);
            pos.unmake_move(m);

            if (score > bestScore) {
                bestScore = score;

                if (score > alpha)
                    alpha = score;
                if (score >= beta)
                    return score;
            }
        }
    }
    
    return bestScore;
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
    nodeCount = 0;
}


void Searcher::print_stats() {
    tt.print_stats();
}


} // namespace harukashogi