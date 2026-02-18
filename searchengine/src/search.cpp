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
    generation = pos.get_move_count();
    tt.new_search(generation);

    if (useOpeningBook) {
        Move bookMove = openingBook.sample_move(pos.get_key());
        if (!bookMove.is_null()) {
            if (logLevel >= INFO)
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

    // lower the move histories by 3/4, to shrink old values and make new values more important
    for (int i = 0; i < NUM_COLORS; i++)
        for (int j = 0; j < HISTORY_SIZE; j++)
            moveHistory[i][j] = moveHistory[i][j] * 3 / 4;

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

    if (logLevel >= INFO)
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
    std::tuple<bool, TTData, TTWriter> result = tt.probe(pos.get_key());
    bool ttHit = std::get<0>(result);
    TTData ttData = std::get<1>(result);
    TTWriter ttWriter = std::get<2>(result);
    // if an entry is found, check if the depth is equal or greater than the current depth
    // if it is, there are 3 possibilities to prune:
    // 1. PV_NODE  (exact score): return the score of the entry
    // 3. CUT_NODE (lower bound): return the score if it is greater than beta (pruned)
    // 2. ALL_NODE (upper bound): return the score if it is lower than alpha (already found better)
    Move ttMove = Move::null();
    if (ttHit) {
        if (ttData.depth >= depth) {
            // case 1: PV_NODE (exact score)
            // if the ply is 0 we risk making an illegal move with a type 1 collision
            if (ttData.type == PV_NODE && ply != 0) {
                return ttData.score;
            }
            // case 2: CUT_NODE (lower bound)
            if (ttData.type == CUT_NODE && ttData.score >= beta)
                return ttData.score;
            // case 3: ALL_NODE (upper bound)
            if (ttData.type == ALL_NODE && ttData.score < alpha)
                return ttData.score;
        }

        ttMove = ttData.bestMove;
    }
    // initialize the node type to ALL_NODE
    // if the entry is pruned, set the node type to CUT_NODE
    // if a score >= alpha, set the node type to PV_NODE
    NodeType nodeType = ALL_NODE;

    if (ply == 0 && ttMove.is_null())
        ttMove = bestMove;

    // null move pruning
    // make a null move and search at reduced depth
    // if the score is greater that beta, prune the search
    int score, searchDepth;
    if (!pos.checkers()) {
        pos.make_null_move();
        searchDepth = depth <= 3 ? 0 : depth - 3;
        try {
            score = -min_max(searchDepth, ply + 1, -beta, -beta + 1);
        } catch (const TimeUpException& e) {
            pos.unmake_null_move();
            throw e;
        }
        pos.unmake_null_move();
        if (score >= beta)
            return score;
    }

    // initialize the move picker
    MovePicker movePicker(pos, depth, moveHistory[pos.side_to_move()], ttMove);

    // loop through children nodes
    int bestScore = -INF_SCORE;
    Move nodeBestMove = Move::null();
    Move m;

    // variables for late move reductions
    int nMoves = 0;
    int reduction;

    while ((m = movePicker.next_move()) != Move::null()) {
        // late move reductions
        // if we have explored more than LMR_N_MOVES moves, lower the depth by 1
        // if the search score returned is higher than alpha, research at full depth
        nMoves++;
        reduction = 1 + std::log(nMoves) * std::log(depth - 1) / 3;
        searchDepth = depth > 2 ? depth - reduction : depth - 1;
        
        pos.make_move(m);
        // in case of time up, unmake the move before stopping
        // the pos must be restored to the original position
        try {
            score = -min_max(searchDepth, ply + 1, -beta, -alpha);
        } catch (const TimeUpException& e) {
            pos.unmake_move(m);
            throw e;
        }
        pos.unmake_move(m);

        // if the score is greater than alpha, and the search depth is less than the full depth
        // perform a search at full depth
        if (score >= alpha && searchDepth < depth - 1) {
            pos.make_move(m);
            try {
                score = -min_max(depth - 1, ply + 1, -beta, -alpha);
            } catch (const TimeUpException& e) {
                pos.unmake_move(m);
                throw e;
            }
            pos.unmake_move(m);
        }

        // update best score and the pv table
        if (score > bestScore) {
            bestScore = score;
            nodeBestMove = m;
            if (ply == 0)
                bestMove = nodeBestMove;
        }

        // alpha-beta pruning
        // if the score is greater than alpha, update alpha
        if (score >= alpha) {
            alpha = score;
            nodeType = PV_NODE;
        }

        // if alpha is greater than beta, prune the search
        // fail soft
        if (alpha >= beta) {
            // update the move history to add a bonus for the move
            if (!m.is_null()) {
                int bonus = depth * depth;
                moveHistory[pos.side_to_move()][m.raw()] << bonus;
            }

            // update the transposition table entry
            ttWriter.write(pos.get_key(), bestScore, nodeBestMove, depth, CUT_NODE, generation);

            return bestScore;
        }
    }

    ttWriter.write(pos.get_key(), bestScore, nodeBestMove, depth, nodeType, generation);

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
    MovePicker movePicker(pos, 0, moveHistory[pos.side_to_move()]);

    // search through the scored captures
    int score;
    Move m;
    while ((m = movePicker.next_move()) != Move::null()) {
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

    // when setting a new position, reset the move history
    for (int i = 0; i < NUM_COLORS; i++)
        for (int j = 0; j < HISTORY_SIZE; j++)
            moveHistory[i][j] = 0;
}


void Searcher::print_stats() {
    tt.print_stats();
}


} // namespace harukashogi