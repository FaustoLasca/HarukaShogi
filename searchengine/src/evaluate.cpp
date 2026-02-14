#include "evaluate.h"
#include "position.h"
#include "types.h"

namespace harukashogi {


constexpr int PawnSquareValues[NUM_COLORS][NUM_SQUARES] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0,
     3, 3, 3, 3, 3, 3, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 3,
     2, 2, 2, 2, 2, 2, 2, 2, 2,
     1, 1, 1, 1, 1, 1, 1, 1, 1,
     0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0},

    {0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 1,
     2, 2, 2, 2, 2, 2, 2, 2, 2,
     3, 3, 3, 3, 3, 3, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 3,
     0, 0, 0, 0, 0, 0, 0, 0, 0}
};



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
    for (PieceType pt = GOLD; pt < NUM_PIECE_TYPES; ++pt)
        score += (popcount(pos.pieces(sideToMove, pt)) - popcount(pos.pieces(~sideToMove, pt)))
                 * 100*PieceValues[pt];

    // add pawn position values
    Bitboard ownPawns = pos.pieces(sideToMove, PAWN);
    while (ownPawns)
        score += PawnSquareValues[sideToMove][pop_lsb(ownPawns)];
    Bitboard oppPawns = pos.pieces(~sideToMove, PAWN);
    while (oppPawns)
        score -= PawnSquareValues[~sideToMove][pop_lsb(oppPawns)];

    // add the value of the king protection
    score -= (popcount(pos.attacks<KING>(sideToMove)  & ~pos.all_pieces(sideToMove))
           -  popcount(pos.attacks<KING>(~sideToMove) & ~pos.all_pieces(~sideToMove)))
           * 3;

    // add sliding piece mobility
    Bitboard sliding = pos.pieces(sideToMove, BISHOP) | pos.pieces(sideToMove, P_BISHOP);
    Bitboard attacks = 0;
    while (sliding)
        attacks |= attacks_bb<BLACK, BISHOP>(pop_lsb(sliding), pos.all_pieces());
    score += popcount(attacks) * 1;

    sliding = pos.pieces(sideToMove, ROOK) | pos.pieces(sideToMove, P_ROOK);
    attacks = 0;
    while (sliding)
        attacks |= attacks_bb<BLACK, ROOK>(pop_lsb(sliding), pos.all_pieces());
    score += popcount(attacks) * 1;

    sliding = pos.pieces(~sideToMove, BISHOP) | pos.pieces(~sideToMove, P_BISHOP);
    attacks = 0;
    while (sliding)
        attacks |= attacks_bb<WHITE, BISHOP>(pop_lsb(sliding), pos.all_pieces());
    score -= popcount(attacks) * 1;

    sliding = pos.pieces(~sideToMove, ROOK) | pos.pieces(~sideToMove, P_ROOK);
    attacks = 0;
    while (sliding)
        attacks |= attacks_bb<WHITE, ROOK>(pop_lsb(sliding), pos.all_pieces());
    score -= popcount(attacks) * 1;

    

    // add the value of the hand pieces
    for (PieceType pt = SILVER; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
        // hand pieces are volued more than board pieces
        score += (pos.hand_count(sideToMove, pt) * 120*PieceValues[pt]);
        score -= (pos.hand_count(~sideToMove, pt) * 120*PieceValues[pt]);
    }

    return score;
}


} // namespace harukashogi