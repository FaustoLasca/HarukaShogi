#include <algorithm>
#include <chrono>
#include <cmath>

#include "search.h"
#include "evaluate.h"
#include "movegen.h"
#include "movepicker.h"


namespace chr = std::chrono;

namespace harukashogi {


int REDUCTION_TABLE[MAX_MOVES][MAX_DEPTH];


void Search::init() {
    for (int nMoves = 0; nMoves < MAX_MOVES; nMoves++) {
        for (int depth = 0; depth < MAX_DEPTH; depth++) {
            REDUCTION_TABLE[nMoves][depth] = std::log(nMoves) * std::log(depth) * 2 / 5;
        }
    }
}


constexpr chr::milliseconds MAX_MOVETIME = chr::milliseconds(2000);
constexpr chr::milliseconds MIN_MOVETIME = chr::milliseconds(10);


void Worker::search() {
    if (is_master()) {
        // the master thread handles the search limits
        // exact search time
        if (limits.moveTime.count() > 0)
            searchTime = limits.moveTime;
        // if there are moves before the first time control, set the stop time to 2 seconds
        else if (limits.movesToGo > 0)
            searchTime = MAX_MOVETIME;
        // with normal time contral
        else if (limits.time[BLACK].count() > 0 || limits.time[WHITE].count() > 0 ||
                 limits.inc[BLACK].count()  > 0 || limits.inc[WHITE].count() > 0  ||
                 limits.byoyomi.count() > 0) {
            chr::milliseconds time = limits.time[rootPos.side_to_move()];
            chr::milliseconds inc = limits.inc[rootPos.side_to_move()];

            searchTime = std::max(time/30 + inc/2, limits.byoyomi) - moveOverhead;
            searchTime = std::clamp(searchTime, MIN_MOVETIME, MAX_MOVETIME);
        }
        // with infinite time control
        else
            searchTime = chr::minutes(10);

        if (!limits.ponder)
            stopTime = limits.startTime + searchTime;

        // inform the transposition table that a new search has started
        tt.new_search();

        // start the searching of the slaves
        threads.slaves_start_searching();
    }
    
    info = SearchInfo();
    empty_stack();
    searchPos.set(rootPos.sfen());
    try {
        iterative_deepening();
    } catch (const AbortSearchException& e) {}

    // the master thread waits for the slaves to finish searching and collects the results
    if (is_master()) {
        
        // keep searching if still pondering or infinite time control
        // busy wait loop
        while ((limits.infinite && !stop) || (limits.ponder && !ponderhit && !stop)) {}

        // wait for the slaves to finish searching
        // if thread has geen stopped in an infinite or pondering search, abort the search
        // for the slaves (edge case)
        if (stop || ponderhit) {
            threads.abort_search();
        }
        threads.wait_search_finished_slaves();

        // don't output the best move if stopping a pondering search
        const Worker& bestThread = get_best_thread();
        outputManager.on_iter(bestThread.info);
        outputManager.on_best_move(bestThread.info.pv[0], bestThread.info.pv[1]);
    }
}


const Worker& Worker::get_best_thread() {
    assert(is_master());

    Move moveList[MAX_MOVES];
    Move* end = generate<LEGAL>(rootPos, moveList);

    int votes[MAX_MOVES] = {};
    for (size_t i = 0; i < threads.size(); i++) {
        Move* found = std::find(moveList, end, threads[i].info.pv[0]);
        if (found != end)
            votes[found - moveList] += 1 << threads[i].info.depth;
    }

    int bestVote = -1;
    const Worker* bestThread = threads.begin()->get();
    for (auto& thread : threads) {
        Move* found = std::find(moveList, end, thread->info.pv[0]);
        if (found == end)
            continue;
        int v = votes[found - moveList];
        if (v > bestVote) {
            bestVote = v;
            bestThread = thread.get();
        }
    }

    return *bestThread;
}


constexpr int ASPIRATION_DELTA = 500;
constexpr int ASPIRATION_DELTA_MULT = 100;

void Worker::iterative_deepening() {
    // shrink old values of the move history to make new values more important
    for (int i = 0; i < NUM_COLORS; i++)
        for (int j = 0; j < HISTORY_SIZE; j++) {
            moveHistory[i][j] = moveHistory[i][j] / 8;
            // noise to slightly change the threads' move ordering
            if (j%64 == threadId) {
                moveHistory[i][j] = moveHistory[i][j] + 1;
            }
        }
    
    int old_score = q_search();
    int score;
    int alpha, beta;
    int deltaMult = 1;

    int startingDepth = 1 + threadId % 4;
    
    // loop through the depths
    for (int depth = startingDepth; depth <= MAX_DEPTH; depth++) {
        // aspiration window loop
        while (true) {
            alpha = std::max(old_score - ASPIRATION_DELTA * deltaMult, -INF_SCORE);
            beta  = std::min(old_score + ASPIRATION_DELTA * deltaMult,  INF_SCORE);
            score = search<ROOT_NODE>(stack, depth, alpha, beta);

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


template <NodeType searchType>
int Worker::search(StackEntry* stack, int depth, int alpha, int beta) {
    stack->pv.fill(Move::null());

    // if the depth is 0, return the evaluation of the position
    if (searchPos.is_game_over() || depth == 0)
        return q_search(alpha, beta);
        
    // throws an exception if the time is up
    stop_check();

    info.nodeCount++;

    constexpr NodeType nodeType = searchType == ROOT_NODE ? PV_NODE : searchType;

    // probe the transposition table for an entry
    std::tuple<bool, TTData, TTWriter> result = tt.probe(searchPos.get_key());
    bool ttHit = std::get<0>(result);
    TTData ttData = std::get<1>(result);
    TTWriter ttWriter = std::get<2>(result);
    // if an entry is found, check if the depth is equal or greater than the current depth
    // if it is, there are 3 possibilities to prune:
    // 1. PV_ENTRY  (exact score): return the score of the entry
    // 3. CUT_ENTRY (lower bound): return the score if it is greater than beta (pruned)
    // 2. ALL_ENTRY (upper bound): return the score if it is lower than alpha (already found better)
    Move ttMove = Move::null();
    if (ttHit) {
        if (ttData.depth >= depth && nodeType != PV_NODE) {
            // case 1: PV_ENTRY (exact score)
            // if the ply is 0 we risk making an illegal move with a type 1 collision
            if (ttData.type == PV_ENTRY)
                return ttData.score;
            // case 2: CUT_ENTRY (lower bound)
            if (ttData.type == CUT_ENTRY && ttData.score >= beta)
                return ttData.score;
            // case 3: ALL_ENTRY (upper bound)
            if (ttData.type == ALL_ENTRY && ttData.score < alpha)
                return ttData.score;
        }

        ttMove = ttData.bestMove;
    }

    // initialize the node type to ALL_NODE
    // if the entry is pruned, set the node type to CUT_NODE
    // if a score >= alpha, set the node type to PV_NODE
    TTEntryType entryType = ALL_ENTRY;

    if (searchType == ROOT_NODE && ttMove.is_null())
        ttMove = info.pv[0];

    // null move pruning
    // make a null move and search at reduced depth
    // if the score is greater that beta, prune the search
    int searchDepth, score;
    if (!searchPos.checkers()) {
        searchPos.make_null_move();
        searchDepth = depth <= 3 ? 0 : depth - 3;
        score = -search<NON_PV_NODE>(stack+1, searchDepth, -beta, -beta + 1);
        searchPos.unmake_null_move();
        if (score >= beta)
            return score;
    }

    // initialize the move picker
    MovePicker movePicker(searchPos, depth, moveHistory[searchPos.side_to_move()], ttMove);

    // loop through children nodes
    int bestScore = -INF_SCORE;
    Move m;

    // variables for late move reductions
    int nMoves = 0;
    int reduction;

    while ((m = movePicker.next_move()) != Move::null()) {
        // late move reductions
        // if we have explored more than LMR_N_MOVES moves, lower the depth by 1
        // if the search score returned is higher than alpha, research at full depth
        nMoves++;
        if (depth > 2 && !searchPos.gives_check(m) && !searchPos.checkers()) {
            reduction = 1 + REDUCTION_TABLE[nMoves][depth-1];
        }
        else reduction = 1;
        
        searchDepth = depth - reduction;
        
        searchPos.make_move(m);
        // Princpal Variation Search (PVS)
        // If we are in a PV node, search only the first node with a full alpha beta window,
        // the other moves are searched as NON_PV nodes with null window between alpha and alpha+1.
        // If the search fails high (and thus returns a score > alpha), research with a full window.
        if (nMoves == 1) {
            score = -search<nodeType>(stack+1, depth - 1, -beta, -alpha);
        }
        else {
            score = -search<NON_PV_NODE>(stack+1, searchDepth, -alpha-1, -alpha);
            // If the search fails high and the searchdepth was reduced,
            // first search at full depth while keeping the null window.
            if (score > alpha && searchDepth < depth - 1) {
                score = -search<NON_PV_NODE>(stack+1, depth - 1, -alpha-1, -alpha);
            }
            // If it still fails high in a PV node, search at full depth with a full window.
            if (score > alpha && nodeType == PV_NODE) {
                score = -search<PV_NODE>(stack+1, depth - 1, -beta, -alpha);
            }
        }
        searchPos.unmake_move(m);

        // update best score and the pv table
        if (score > bestScore) {
            bestScore = score;
            if constexpr (nodeType == PV_NODE) {
                stack->pv[0] = m;
                std::copy((stack+1)->pv.begin(), (stack+1)->pv.end()-1, stack->pv.begin() + 1);
            }
            // if the search is root, update the best move and evaluation of the worker
            if constexpr (searchType == ROOT_NODE) {
                info.pv = stack->pv;
                info.eval = bestScore;
            }
                
        }

        // alpha-beta pruning
        // if the score is greater than alpha, update alpha
        if (score >= alpha) {
            alpha = score;
            entryType = PV_ENTRY;
        }

        // if alpha is greater than beta, prune the search
        // fail soft
        if (alpha >= beta) {
            // update the move history to add a bonus for the move
            if (!m.is_null()) {
                int bonus = depth * depth;
                moveHistory[searchPos.side_to_move()][m.raw()] << bonus;
            }

            // update the transposition table entry
            ttWriter.write(searchPos.get_key(), bestScore, stack->pv[0], depth, CUT_ENTRY);

            return bestScore;
        }
    }

    ttWriter.write(searchPos.get_key(), bestScore, stack->pv[0], depth, entryType);

    return bestScore;
}


int Worker::q_search(int alpha, int beta) {
    info.nodeCount++;

    // throws an exception to abort the search
    if (is_search_aborted())
        throw AbortSearchException();

    int eval = evaluate(searchPos);

    if (eval >= beta)
        return eval;
    if (searchPos.is_game_over())
        return eval;

    int bestScore = eval;
    if (eval > alpha)
        alpha = eval;

    // initialize the move picker
    MovePicker movePicker(searchPos, 0, moveHistory[searchPos.side_to_move()]);

    // search through the scored captures
    int score;
    Move m;
    while ((m = movePicker.next_move()) != Move::null()) {
        searchPos.make_move(m);
        score = -q_search(-beta, -alpha);
        searchPos.unmake_move(m);

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

    if (!is_master())
        return;

    if (stop.load(std::memory_order_relaxed)) {
        threads.abort_search();
        throw AbortSearchException();
    }

    if (limits.ponder && ponderhit.load(std::memory_order_relaxed)) {
        limits.ponder = false;
        stopTime = chr::steady_clock::now() + searchTime;
    }

    if (limits.nodes > 0 && info.nodeCount >= limits.nodes) {
        std::cout << "nodes limit reached" << std::endl;
        std::cout << "node count:  " << info.nodeCount << std::endl;
        std::cout << "nodes limit: " << limits.nodes << std::endl;
        threads.abort_search();
        throw AbortSearchException();
    }
    
    // time up
    if (!limits.infinite && !limits.ponder && chr::steady_clock::now() >= stopTime) {
        threads.abort_search();
        throw AbortSearchException();
    }
}


void Worker::clear() {
    // clear the main history
    for (int i = 0; i < NUM_COLORS; i++)
        for (int j = 0; j < HISTORY_SIZE; j++)
            moveHistory[i][j] = 0;
}


void Worker::set_position(std::string sfen) {
    rootPos.set(sfen);
    searchPos.set(sfen);
}


void Worker::empty_stack() {
    for (int i = 0; i < MAX_DEPTH; i++) {
        stack[i].ply = i;
        stack[i].pv.fill(Move::null());
    }
}


} // namespace harukashogi