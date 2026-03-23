#ifndef MOVEPICKER_H
#define MOVEPICKER_H

#include "movegen.h"
#include "position.h"
#include "history.h"

namespace harukashogi {


class ValMove : public Move {
    public:
        int value;

        void operator=(Move& move) {
            this->data = move.raw();
        };
};


enum Stage {
    // normal search stages
    TT_STAGE,
    CAPTURE_STAGE_INIT,
    GOOD_CAPTURE_STAGE,
    QUIET_STAGE_INIT,
    QUIET_STAGE,
    BAD_CAPTURE_STAGE,

    // evasion
    EVASION_TT_STAGE,
    EVASION_STAGE_INIT,
    EVASION_STAGE,

    // quiescence
    QUIESCENCE_STAGE_INIT,
    QUIESCENCE_STAGE
};


class MovePicker {
    public:
        MovePicker(Position& pos, int depth, HistoryEntry* moveHistory, Move ttMove = Move::null());

        Move next_move();

    private:
        template <Stage stage>
        ValMove* score(ValMove* scoredMoves, Move* moveList, Move* end);
    
        Position& pos;
        int depth;
        Move ttMove;

        int stage;

        HistoryEntry* moveHistory;

        ValMove moves[MAX_MOVES];
        ValMove *curr, *capturesEnd, *badCapturesEnd, *movesEnd;
};


} // namespace harukashogi
#endif // MOVEPICKER_H