#include <algorithm>
#include <chrono>
#include <cmath>

#include "search.h"
#include "evaluate.h"
#include "movegen.h"
#include "movepicker.h"
#include "misc.h"


namespace chr = std::chrono;

namespace harukashogi {


void Worker::search() {
    if (is_master()) {
        // the master thread handles the search limits
        if (limits.moveTime.count() > 0)
            stopTime = limits.startTime + limits.moveTime;
        // TODO: handle the other limits
        else
            stopTime = limits.startTime + chr::seconds(5);

        // inform the transposition table that a new search has started
        tt.new_search();

        // start the searching of the slaves
        threads.slaves_start_searching();
    }
    
    info = SearchInfo();
    pos.set(rootPos.sfen());
    try {
        iterative_deepening();
    } catch (const AbortSearchException& e) {}

    // the master thread waits for the slaves to finish searching and collects the results
    if (is_master()) {
        threads.wait_search_finished_slaves();
        
        // threads vote for the best move
        // generate the legal moves to constrain votes to valid moves only, to avoid illegal moves
        // from corrupted TT reads (rare but possible)
        Move bestMove;
        Move moveList[MAX_MOVES];
        Move* end = generate<LEGAL>(rootPos, moveList);
        int nLegal = end - moveList;

        int votes[MAX_MOVES] = {};
        for (auto& thread : threads) {
            Move* found = std::find(moveList, end, thread->info.bestMove);
            if (found != end)
                votes[found - moveList] += 1 << thread->info.depth;
        }

        // get the best move depending on the vote values
        bestMove = moveList[std::max_element(votes, votes + nLegal) - votes];

        outputManager.on_best_move(bestMove, Move::null());
    }
}


constexpr int ASPIRATION_DELTA = 500;
constexpr int ASPIRATION_DELTA_MULT = 100;

void Worker::iterative_deepening() {
    // lower the move histories by 3/4, to shrink old values and make new values more important
    for (int i = 0; i < NUM_COLORS; i++)
        for (int j = 0; j < HISTORY_SIZE; j++) {
            moveHistory[i][j] = moveHistory[i][j] * 3 / 4;
            if (j%64 == threadId) {
                moveHistory[i][j] = moveHistory[i][j] + 1;
            }
        }
    
    int old_score = q_search();
    int score;
    int alpha, beta;
    int deltaMult = 1;
    
    // loop through the depths
    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        // aspiration window loop
        while (true) {
            alpha = std::max(old_score - ASPIRATION_DELTA * deltaMult, -INF_SCORE);
            beta  = std::min(old_score + ASPIRATION_DELTA * deltaMult,  INF_SCORE);
            score = search<true>(depth, 0, alpha, beta);

            // if the score is within the aspiration window, break the loop
            if (score > alpha && score < beta)
                break;

            // otherwise, increase the delta multiplier by a factor of 2 and search again
            deltaMult *= ASPIRATION_DELTA_MULT;
        }

        // set the necessary values for the next iteration of the aspiration window loop
        old_score = score;
        deltaMult = 1;

        info.depth = depth;

        // the master thread outputs the search info
        if (is_master()) {
            outputManager.on_iter(info);
        }
    }
}


template <bool isRoot>
int Worker::search(int depth, int ply, int alpha, int beta) {

    // if the depth is 0, return the evaluation of the position
    if (pos.is_game_over() || depth == 0)
        return q_search(alpha, beta);
        
    // throws an exception if the time is up
    stop_check();

    info.nodeCount++;

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
            if (!isRoot && ttData.type == PV_NODE)
                return ttData.score;
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

    if (isRoot && ttMove.is_null())
        ttMove = info.bestMove;

    // null move pruning
    // make a null move and search at reduced depth
    // if the score is greater that beta, prune the search
    int searchDepth, score;
    if (!pos.checkers()) {
        pos.make_null_move();
        searchDepth = depth <= 3 ? 0 : depth - 3;
        score = -search<false>(searchDepth, ply + 1, -beta, -beta + 1);
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
        reduction = 1 + std::log(nMoves) * std::log(depth - 1) * 2 / 5;
        searchDepth = depth > 2 ? depth - reduction : depth - 1;
        
        pos.make_move(m);
        score = -search<false>(searchDepth, ply + 1, -beta, -alpha);
        pos.unmake_move(m);

        // if the score is greater than alpha, and the search depth is less than the full depth
        // perform a search at full depth
        if (score >= alpha && searchDepth < depth - 1) {
            pos.make_move(m);
            score = -search<false>(depth - 1, ply + 1, -beta, -alpha);
            pos.unmake_move(m);
        }

        // update best score and the pv table
        if (score > bestScore) {
            bestScore = score;
            nodeBestMove = m;
            // if the search is root, update the best move and evaluation of the worker
            if constexpr (isRoot) {
                info.bestMove = nodeBestMove;
                info.eval = bestScore;
            }
                
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
            ttWriter.write(pos.get_key(), bestScore, nodeBestMove, depth, CUT_NODE);

            return bestScore;
        }
    }

    ttWriter.write(pos.get_key(), bestScore, nodeBestMove, depth, nodeType);

    return bestScore;
}


int Worker::q_search(int alpha, int beta) {
    info.nodeCount++;

    // throws an exception to abort the search
    if (is_search_aborted())
        throw AbortSearchException();

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
        score = -q_search(-beta, -alpha);
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


void Worker::stop_check() {
    if (is_search_aborted())
        throw AbortSearchException();

    // the master thread aborts the search for all threads if the time is up
    if (is_master() && !limits.infinite && chr::steady_clock::now() >= stopTime) {
        threads.abort_search();
        throw AbortSearchException();
    }
}


void Worker::set_position(std::string sfen) {
    rootPos.set(sfen);

    // when setting a new position, reset the move history
    for (int i = 0; i < NUM_COLORS; i++)
        for (int j = 0; j < HISTORY_SIZE; j++)
            moveHistory[i][j] = 0;
}


void SearchManager::set_position(std::string sfen) {
    pos.set(sfen);

    for (auto& thread : threads)
        thread->set_position(sfen);
}


SearchInfo SearchManager::get_results() {
    assert(!threads.is_searching());
    
    SearchInfo results;

    for (auto& thread : threads) {
        // sum of the node counts of all threads
        results.nodeCount += thread->info.nodeCount;
        // average of the evaluations of all threads
        // (not accurate but close enough)
        results.eval += thread->info.eval;
        // max depth of all threads
        results.depth = std::max(results.depth, thread->info.depth);
    }
    results.eval /= threads.size();

    // threads vote for the best move
    // generate the legal moves to constrain votes to valid moves only, to avoid illegal moves
    // from corrupted TT reads (rare but possible)
    Move moveList[MAX_MOVES];
    Move* end = generate<LEGAL>(pos, moveList);
    int nLegal = end - moveList;

    int votes[MAX_MOVES] = {};
    // std::cout << "results:" << std::endl;
    for (auto& thread : threads) {
        Move* found = std::find(moveList, end, thread->info.bestMove);
        if (found != end)
            votes[found - moveList] += 1 << thread->info.depth;

        // std::cout << "depth: " << thread->info.depth
        //           << " - move: " << thread->info.bestMove 
        //           << " - eval: " << thread->info.eval << std::endl;
    }

    // get the best move depending on the vote values
    if (nLegal > 0)
        results.bestMove = moveList[std::max_element(votes, votes + nLegal) - votes];

    return results;
}


void SearchManager::print_stats() {
    tt.print_stats();
}


// void Searcher::set_position(std::string sfen) {
//     searchManager.set_position(sfen);
//     pos.set(sfen);
// }


// Move Searcher::search(chr::milliseconds timeLimit, int depth) {
//     if (useOpeningBook) {
//         Move move = openingBook.sample_move(pos.get_key());
//         if (!move.is_null())
//             return move;
//     }

//     searchManager.start_searching();
//     std::this_thread::sleep_for(timeLimit);
//     searchManager.abort_search();
//     searchManager.wait_search_finished();
//     SearchInfo results =  searchManager.get_results();
//     // std::cout << "eval: " << results.eval 
//     //           << " - depth: " << results.depth 
//     //           << " - node count: " << results.nodeCount << std::endl;
//     return results.bestMove;
// }


// std::string Searcher::search(int timeLimit, int depth) {
//     return search(chr::milliseconds(timeLimit), depth).to_string();
// }

// void Searcher::print_stats() {
//     std::cout << "TT stats:   " << std::endl;
//     searchManager.print_stats();
// }


} // namespace harukashogi