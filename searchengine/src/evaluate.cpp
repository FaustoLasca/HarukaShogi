#include "evaluate.h"
#include "position.h"
#include "types.h"

namespace harukashogi {


constexpr int PieceValues[NUM_PIECE_TYPES] = {
    0, 6000, 5000, 4000, 3000, 10000, 12000, 1000,
    6000, 6000, 6000, 16000, 16000, 6000
};


int evaluate(Position& pos) {
    Color sideToMove = pos.side_to_move();
    int moveCount = pos.get_move_count();
    int score = 0;

    // if the game is over, return the winning score
    if (pos.is_game_over())
        // if the game is over, the side to move has lost
        return -WIN_SCORE + moveCount;

    // add the piece values to the score
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (pos.piece(sq) != NO_PIECE)
            score += (color_of(pos.piece(sq)) == sideToMove) ? 
                PieceValues[type_of(pos.piece(sq))] : -PieceValues[type_of(pos.piece(sq))];
    }

    // add the value of the hand pieces
    for (PieceType pt = SILVER; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
        score += (pos.hand_count(sideToMove, pt) * PieceValues[pt]);
        score -= (pos.hand_count(~sideToMove, pt) * PieceValues[pt]);
    }

    // add a small penalty for the number of moves
    // for equivalent outcomes, the direction with less moves is preferred
    score += (score >= 0) ? -moveCount : moveCount;

    return score;
}


} // namespace harukashogi