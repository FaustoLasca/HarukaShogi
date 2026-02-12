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
        Bitboard own = pos.pieces(sideToMove, pt);
        Bitboard their = pos.pieces(~sideToMove, pt);
        score += (popcount(own) - popcount(their)) * 100*PieceValues[pt];
    }

    // add the value of the king protection
    Bitboard kingProtection = attacks_bb<BLACK, KING>(pos.king_square(sideToMove))
                          & pos.all_pieces(sideToMove);
    score += popcount(kingProtection) * 2;

    kingProtection = attacks_bb<BLACK, KING>(pos.king_square(~sideToMove))
                          & pos.all_pieces(~sideToMove);
    score -= popcount(kingProtection) * 2;

    // add sliding piece mobility
    Bitboard bishops = pos.pieces(sideToMove, BISHOP) | pos.pieces(sideToMove, P_BISHOP);
    while (bishops) {
        Square from = pop_lsb(bishops);
        Bitboard mobility = sld_attacks_bb<BLACK, BISHOP>(from, pos.all_pieces());
        score += popcount(mobility) * 1;
    }
    Bitboard rooks = pos.pieces(sideToMove, ROOK) | pos.pieces(sideToMove, P_ROOK);
    while (rooks) {
        Square from = pop_lsb(rooks);
        Bitboard mobility = sld_attacks_bb<BLACK, ROOK>(from, pos.all_pieces());
        score += popcount(mobility) * 1;
    }

    // remove opponent sliding piece mobility
    bishops = pos.pieces(~sideToMove, BISHOP) | pos.pieces(~sideToMove, P_BISHOP);
    while (bishops) {
        Square from = pop_lsb(bishops);
        Bitboard mobility = sld_attacks_bb<BLACK, BISHOP>(from, pos.all_pieces());
        score -= popcount(mobility) * 1;
    }
    rooks = pos.pieces(~sideToMove, ROOK) | pos.pieces(~sideToMove, P_ROOK);
    while (rooks) {
        Square from = pop_lsb(rooks);
        Bitboard mobility = sld_attacks_bb<BLACK, ROOK>(from, pos.all_pieces());
        score -= popcount(mobility) * 1;
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