#include <algorithm>

#include "movepicker.h"

namespace harukashogi {


MovePicker::MovePicker(Position& pos, int depth, Move ttMove) : pos(pos), depth(depth), ttMove(ttMove) {
    // initialize the score moves array as empty
    curr = scoredMoves;
    scoredEnd = scoredMoves;

    // check if the tt move is legal
    if (pos.is_pseudo_legal(ttMove)) {
        if (pos.is_legal(ttMove)) {
            if (pos.checkers())
                stage = EVASION_TT_STAGE;
            else
                stage = TT_STAGE;
        }
    }
 
    // if the depth is 0 or less, we are in quiescence
    else if (depth <= 0)
        stage = QUIESCENCE_STAGE;
    // if there are checkers, we are in evasion
    else if (pos.checkers())
        stage = EVASION_STAGE;
    // otherwise, we are in capture search
    else
        stage = CAPTURE_STAGE;
}


// takes a movelist and returns fills the scoredMoves array with the moves and their scores
ValMove* MovePicker::score(ValMove* scoredMoves, Move* moveList, Move* end) {
    ValMove valMove;

    ValMove* curr = scoredMoves;
    for (Move* m = moveList; m < end; ++m) {
        valMove = *m;
        valMove.value = evaluate_move(pos, *m);
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

        case CAPTURE_STAGE_INIT:
        case QUIESCENCE_STAGE_INIT: {
            stage++;
            Move moveList[MAX_MOVES];
            Move* end = generate<CAPTURES>(pos, moveList);
            scoredEnd = score(scoredMoves, moveList, end);
            std::sort(scoredMoves, scoredEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });
            break;
        }

        case QUIET_STAGE_INIT: {
            stage++;
            Move moveList[MAX_MOVES];
            Move* end = generate<QUIET>(pos, moveList);
            scoredEnd = score(scoredMoves, moveList, end);
            std::sort(scoredMoves, scoredEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });
            break;
        }

        case EVASION_STAGE_INIT: {
            stage++;
            Move moveList[MAX_MOVES];
            Move* end = generate<EVASIONS>(pos, moveList);
            scoredEnd = score(scoredMoves, moveList, end);
            std::sort(scoredMoves, scoredEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });
            break;
        }
    }
    
    // move curr to the next legal move
    do {
        curr++;
    } while (curr < scoredEnd && !pos.is_legal(*curr));

    if (curr == scoredEnd) {
        // if the moves are over, return null move
        if (is_last_stage(Stage(stage))) {
            return Move::null();
        }
        // otherwise, move to the next stage and return the next move
        else {
            stage++;
            return next_move();
        }
    }

    return *curr;
}


} // namespace harukashogi