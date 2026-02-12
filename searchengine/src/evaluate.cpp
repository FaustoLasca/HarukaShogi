#include "evaluate.h"
#include "position.h"
#include "types.h"

namespace harukashogi {


int evaluate(Position& pos) {
    Color sideToMove = pos.side_to_move();
    int score = 0;

    // if the game is over, return the winning score
    if (pos.is_game_over()) {
        // if the game is over, the side to move has lost
        if (pos.get_winner() == NO_COLOR)
            return 0;
        else
            return -WIN_SCORE;
    }
        

    // add the piece values to the score
    for (PieceType pt = GOLD; pt < NUM_PIECE_TYPES; ++pt) {
        score += (popcount(pos.pieces(sideToMove, pt)) - popcount(pos.pieces(~sideToMove, pt))) * 100*PieceValues[pt];
    }

    // add the value of the hand pieces
    for (PieceType pt = SILVER; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
        // hand pieces are volued more than board pieces
        score += (pos.hand_count(sideToMove, pt) * 120*PieceValues[pt]);
        score -= (pos.hand_count(~sideToMove, pt) * 120*PieceValues[pt]);
    }

    return score;
}


} // namespace harukashogi