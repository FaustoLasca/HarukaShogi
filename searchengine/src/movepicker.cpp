#include <algorithm>

#include "movepicker.h"
#include "evaluate.h"

namespace harukashogi {


MovePicker::MovePicker(Position& pos, int depth, Move ttMove) : pos(pos), depth(depth), ttMove(ttMove) {
    // initialize the score moves array as empty
    curr = scoredMoves;
    scoredEnd = scoredMoves;

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
    else if (depth <= 0)
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
ValMove* MovePicker::score(ValMove* scoredMoves, Move* moveList, Move* end) {
    ValMove valMove;

    ValMove* curr = scoredMoves;
    for (Move* m = moveList; m < end; ++m) {
        valMove = *m;
        int score = 0;
        
        
        if (!m->is_drop()) {
            if (pos.is_capture(*m))
                score += 1000*PieceValues[type_of(pos.piece(m->to()))] - 
                        100*PieceValues[type_of(pos.piece(m->from()))];

            // if the move is a promotion, add a bonus to the score
            // based on the value of the promoted piece
            if (m->is_promotion()) {
                PieceType promotedPT = type_of(promote_piece(pos.piece(m->from())));
                score += 1000 * (PieceValues[promotedPT] - 
                        PieceValues[type_of(pos.piece(m->from()))] );
            }
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

        case CAPTURE_STAGE_INIT:
        case QUIESCENCE_STAGE_INIT: {
            stage++;
            Move moveList[MAX_MOVES];
            Move* end = generate<CAPTURES>(pos, moveList);
            curr = scoredMoves;
            scoredEnd = score(scoredMoves, moveList, end);
            std::sort(scoredMoves, scoredEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });

            // std::cout << "Captures stage: " << stage << std::endl;
            // std::cout << "N moves: " << end - moveList << std::endl;
            // std::cout << "Score moves: " << scoredEnd - scoredMoves << std::endl;

            break;
        }

        case QUIET_STAGE_INIT: {
            stage++;
            Move moveList[MAX_MOVES];
            Move* end = generate<QUIET>(pos, moveList);
            curr = scoredMoves;
            scoredEnd = score(scoredMoves, moveList, end);
            std::sort(scoredMoves, scoredEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });

            // std::cout << "Quiets stage: " << stage << std::endl;
            // std::cout << "N moves: " << end - moveList << std::endl;
            // std::cout << "Score moves: " << scoredEnd - scoredMoves << std::endl;

            break;
        }

        case EVASION_STAGE_INIT: {
            stage++;
            Move moveList[MAX_MOVES];
            Move* end = generate<EVASIONS>(pos, moveList);
            curr = scoredMoves;
            scoredEnd = score(scoredMoves, moveList, end);
            std::sort(scoredMoves, scoredEnd, [](const ValMove& a, const ValMove& b) {
                return a.value > b.value;
            });

            // std::cout << "Evasion stage: " << stage << std::endl;
            // std::cout << "N moves: " << end - moveList << std::endl;
            // std::cout << "Score moves: " << scoredEnd - scoredMoves << std::endl;

            break;
        }
    }
    
    // move curr to the next legal move
    while (curr < scoredEnd && !pos.is_legal(*curr)) {
        curr++;
    }

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

    Move move = *curr;
    curr++;
    return move;
}


} // namespace harukashogi