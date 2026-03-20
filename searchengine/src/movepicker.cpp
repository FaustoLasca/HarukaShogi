#include <algorithm>

#include "movepicker.h"
#include "misc.h"

namespace harukashogi {


MovePicker::MovePicker(Position& pos, int depth, HistoryEntry* moveHistory, Move ttMove) : 
        pos(pos), depth(depth), moveHistory(moveHistory), ttMove(ttMove) {
    // initialize the score moves array as empty
    curr = moves;
    movesEnd = moves;

    // check if the tt move is legal
    if (pos.is_pseudo_legal(ttMove)) {
        if (pos.is_legal(ttMove)) {
            if (pos.checkers()) {
                stage = EVASION_TT_STAGE;
                return;
            }
            else{
                stage = TT_STAGE;
                return;
            }
        }
    }
 
    // if the depth is 0 or less, we are in quiescence
    if (depth <= 0)
        stage = QUIESCENCE_STAGE_INIT;
    // if there are checkers, we are in evasion
    else if (pos.checkers())
        stage = EVASION_STAGE_INIT;
    // otherwise, we are in capture search
    else
        stage = CAPTURE_STAGE_INIT;

    // std::cout << "Initial stage: " << stage << std::endl;
}


// takes a movelist and returns fills the scoredMoves array with the moves and their scores
template <Stage stage>
ValMove* MovePicker::score(ValMove* scoredMoves, Move* moveList, Move* end) {
    ValMove valMove;

    ValMove* curr = scoredMoves;
    for (Move* m = moveList; m < end; ++m) {
        valMove = *m;
        int score = 0;

        // the generated moves are captures, add the difference in piece values to the score
        // this encourages capturing the most valuable piece with the least valuable piece
        if constexpr (stage == CAPTURE_STAGE_INIT) {
            assert(pos.is_capture(*m));
            score += 100*PieceValues[type_of(pos.piece(m->to()))]
                   - 10* PieceValues[type_of(pos.piece(m->from()))];
        }

        // if we are in the evasion stage, we prioritize captures using the same logic as in the
        // capture stage
        if constexpr (stage == EVASION_STAGE_INIT) {
            if (pos.is_capture(*m)) {
                score += 100*PieceValues[type_of(pos.piece(m->to()))]
                       - 10* PieceValues[type_of(pos.piece(m->from()))];
            }
        }

        // quiet moves are ordered by the history value of the move
        if constexpr (stage == QUIET_STAGE_INIT) {
            score += moveHistory[m->raw()];
        }

        valMove.value = score;
        *curr++ = valMove;
    }

    return curr;
}


// returns the next move to take during search
Move MovePicker::next_move() {
    switch (stage) {
        case TT_STAGE:
        case EVASION_TT_STAGE:
            stage++;
            return ttMove;
        

        // normal search
        case CAPTURE_STAGE_INIT: {
            stage++;

            Move moveList[MAX_MOVES];
            Move *end = generate<CAPTURES>(pos, moveList);
            capturesEnd = score<CAPTURE_STAGE_INIT>(moves, moveList, end);
            curr = moves;

            std::sort(moves, capturesEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });

            [[fallthrough]];
        }

        case GOOD_CAPTURE_STAGE:
            for (; curr < capturesEnd; curr++)
                if (pos.is_legal(*curr) && *curr != ttMove && pos.see_ge(*curr, 0))
                    return *curr++;
            
            stage++;
            [[fallthrough]];

        case QUIET_STAGE_INIT: {
            stage++;

            Move moveList[MAX_MOVES];
            Move* end = generate<QUIET>(pos, moveList);
            movesEnd = score<QUIET_STAGE_INIT>(capturesEnd, moveList, end);
            curr = capturesEnd;
            std::sort(capturesEnd, movesEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });

            [[fallthrough]];
        }

        case QUIET_STAGE:
            for (; curr < movesEnd; curr++)
                if (pos.is_legal(*curr) && *curr != ttMove)
                    return *curr++;
            
            stage++;
            curr = moves;
            [[fallthrough]];

        case BAD_CAPTURE_STAGE:
            for (; curr < capturesEnd; curr++)
                if (pos.is_legal(*curr) && *curr != ttMove && !pos.see_ge(*curr, 0))
                    return *curr++;
            
            return Move::null();
        

        // evasion search
        case EVASION_STAGE_INIT: {
            stage++;

            Move moveList[MAX_MOVES];
            Move *end = generate<EVASIONS>(pos, moveList);
            movesEnd = score<EVASION_STAGE_INIT>(moves, moveList, end);
            curr = moves;

            std::sort(moves, movesEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });

            [[fallthrough]];
        }

        case EVASION_STAGE:
            for (; curr < movesEnd; curr++)
                if (pos.is_legal(*curr) && *curr != ttMove)
                    return *curr++;
            
            return Move::null();


        // quiescence search
        case QUIESCENCE_STAGE_INIT: {
            stage++;

            Move moveList[MAX_MOVES];
            Move *end;
            if (pos.checkers()) {
                end = generate<EVASIONS>(pos, moveList);
                movesEnd = score<EVASION_STAGE_INIT>(moves, moveList, end);
            }
            else {
                end = generate<CAPTURES>(pos, moveList);
                movesEnd = score<CAPTURE_STAGE_INIT>(moves, moveList, end);
            }
            curr = moves;

            [[fallthrough]];
        }

        case QUIESCENCE_STAGE:
            for (; curr < movesEnd; curr++)
                if (pos.is_legal(*curr) && (pos.checkers() || pos.see_ge(*curr, 0)))
                    return *curr++;
            
            return Move::null();
    }

    assert(false);
    return Move::null();
}


} // namespace harukashogi