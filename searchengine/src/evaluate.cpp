#include "evaluate.h"
#include "position.h"
#include "types.h"

namespace harukashogi {


constexpr int PieceValues[NUM_PIECE_TYPES] = {
    0, 6, 5, 4, 3, 10, 12, 1,
    6, 6, 6, 16, 16, 6
};


int evaluate(Position& pos) {
    Color sideToMove = pos.side_to_move();
    int moveCount = pos.get_move_count();
    int score = 0;

    // if the game is over, return the winning score
    if (pos.is_game_over()) {
        // if the game is over, the side to move has lost
        if (pos.get_winner() == NO_COLOR)
            return 0;
        else
            return -WIN_SCORE + moveCount;
    }
        

    // add the piece values to the score
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (pos.piece(sq) != NO_PIECE)
            score += (color_of(pos.piece(sq)) == sideToMove) ? 
                1000*PieceValues[type_of(pos.piece(sq))] : -1000*PieceValues[type_of(pos.piece(sq))];
    }

    // add the value of the hand pieces
    for (PieceType pt = SILVER; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
        // hand pieces are volued more than board pieces
        score += (pos.hand_count(sideToMove, pt) * 1200*PieceValues[pt]);
        score -= (pos.hand_count(~sideToMove, pt) * 1200*PieceValues[pt]);
    }

    // add a small penalty for the number of moves
    // for equivalent outcomes, the direction with less moves is preferred
    score += (score >= 0) ? -moveCount : moveCount;

    return score;
}


int evaluate_move(const Position& pos, Move move) {
    int score = 0;
    // if the move is a capture, add a bonus to the score
    // based on the value of the captured piece and the piece that made the capture
    if (pos.is_capture(move))
        score += 1000*PieceValues[type_of(pos.piece(move.to()))] - 100*PieceValues[type_of(pos.piece(move.from()))];

    // if the move is a promotion, add a bonus to the score
    // based on the value of the promoted piece
    if (move.is_promotion())
        score += 1000 * (PieceValues[type_of(promote_piece(pos.piece(move.from())))] - PieceValues[type_of(pos.piece(move.from()))] );

    return score;
}


} // namespace harukashogi