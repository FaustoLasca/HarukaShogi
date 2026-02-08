#include "movegen.h"
#include "types.h"
#include "position.h"
#include "bitboard.h"
#include "misc.h"

namespace harukashogi {


template<Color c>
Move* splat_pawn_moves(Position& pos, Move* moveList, Bitboard attacks) {
    while (attacks) {
        Square to = pop_lsb(attacks);
        constexpr int delta = c == BLACK ? -dir_delta(N_DIR) : -dir_delta(S_DIR);
        Square from = to + delta;
        if (promotion_zone(to, c) || promotion_zone(from, c))
            *moveList++ = Move(from, to, true);
        *moveList++ = Move(from, to);
    }

    return moveList;
}


template<Color c, PieceType pt>
Move* splat_moves(Position& pos, Move* moveList, Bitboard attacks, Square from) {
    while (attacks) {
        Square to = pop_lsb(attacks);
        if constexpr (can_promote(pt))
            if (promotion_zone(to, c) || promotion_zone(from, c))
                *moveList++ = Move(from, to, true);
        *moveList++ = Move(from, to);
    }

    return moveList;
}


template<Color c>
Move* generate_pawn_moves(Position& pos, Move* moveList, Bitboard target) {
    Bitboard pawns = pos.pieces(c, PAWN);
    Bitboard attacks = c == BLACK ? dir_attacks_bb<N_DIR>(pawns) : dir_attacks_bb<S_DIR>(pawns);
    attacks &= target;
    moveList = splat_pawn_moves<c>(pos, moveList, attacks);
    return moveList;
}


template<Color c, PieceType pt>
Move* generate_moves(Position& pos, Move* moveList, Bitboard target) {
    Bitboard pieces = pos.pieces(c, pt);
    Square from;

    while (pieces) {
        from = pop_lsb(pieces);
        Bitboard attacks = attacks_bb<c, pt>(from, pos.all_pieces());
        attacks &= target;
        moveList = splat_moves<c, pt>(pos, moveList, attacks, from);
    }

    return moveList;
}


// adds a drop move to the move list if the move is pseudo-legal
template<Color c, PieceType pt>
Move* add_drop(Position& pos, Move* moveList, Square sq) {
    if (pos.hand_count(c, pt) > 0) {

        if constexpr (pt == PAWN || pt == LANCE || pt == KNIGHT) {
            // pawns, knights and lances cannot be dropped on the last rank
            if (rank_of(sq) == (c == BLACK ? R_1 : R_9))
                return moveList;
            // knights cannot be dropped on the second last rank
            else if (pt == KNIGHT && rank_of(sq) == (c == BLACK ? R_2 : R_8))
                return moveList;
        }

        // pawns cannot be dropped on the same file as other pawns
        if constexpr (pt == PAWN)
            if (pos.pawn_on_file(c, file_of(sq)))
                return moveList;

        *moveList++ = Move(pt, sq);
        return moveList;
    }

    return moveList;
}


template<Color c>
Move* generate_drops(Position& pos, Move* moveList, Bitboard target) {
    Bitboard bb = invert(pos.all_pieces()) & target;
    
    while (bb) {
        // get the next free square
        Square sq = pop_lsb(bb);
        // add the drop moves for all piece types
        moveList = add_drop<c, GOLD>(pos, moveList, sq);
        moveList = add_drop<c, BISHOP>(pos, moveList, sq);
        moveList = add_drop<c, ROOK>(pos, moveList, sq);
        moveList = add_drop<c, SILVER>(pos, moveList, sq);
        moveList = add_drop<c, LANCE>(pos, moveList, sq);
        moveList = add_drop<c, KNIGHT>(pos, moveList, sq);
        moveList = add_drop<c, PAWN>(pos, moveList, sq);
    }

    return moveList;
}


template <GenType gt, Color c>
Move* generate_all(Position& pos, Move* moveList) {
    Bitboard checkers = pos.checkers();
    Bitboard target;
    Square ksq = pos.king_square(c);
    
    // if there is more than one checker, there can only be king moves
    if (!( gt == EVASIONS && !one_bit(checkers))) {

        target = gt == EVASIONS     ? checkers | between_bb(ksq, lsb(checkers))
               : gt == NON_EVASIONS ? ~pos.all_pieces(c)
               : gt == CAPTURES     ? pos.all_pieces(~c)
                                    : ~pos.all_pieces(); // QUIETS

        moveList = generate_pawn_moves<c>(pos, moveList, target);

        moveList = generate_moves<c, GOLD>(pos, moveList, target);
        moveList = generate_moves<c, BISHOP>(pos, moveList, target);
        moveList = generate_moves<c, ROOK>(pos, moveList, target);
        moveList = generate_moves<c, SILVER>(pos, moveList, target);
        moveList = generate_moves<c, LANCE>(pos, moveList, target);
        moveList = generate_moves<c, KNIGHT>(pos, moveList, target);
        moveList = generate_moves<c, P_BISHOP>(pos, moveList, target);
        moveList = generate_moves<c, P_ROOK>(pos, moveList, target);

        if constexpr (gt != CAPTURES) {
            moveList = generate_drops<c>(pos, moveList, target);
        }

    }

    // treat king moves separately, as the logic is different for EVASION
    Bitboard kingBb = dir_attacks_bb<c, KING>(pos.king_square(c));
    kingBb &= gt == EVASIONS ? ~pos.all_pieces(c) : target; 
    moveList = splat_moves<c, KING>(pos, moveList, kingBb, pos.king_square(c));

    return moveList;
}


template <GenType gt>
Move* generate(Position& pos, Move* moveList) {
    static_assert(gt != LEGAL, "LEGAL is not a valid generation type");
    
    moveList = pos.side_to_move() == BLACK ? generate_all<gt, BLACK>(pos, moveList)
                                           : generate_all<gt, WHITE>(pos, moveList);

    return moveList;
}


template Move* generate<EVASIONS>(Position& pos, Move* moveList);
template Move* generate<NON_EVASIONS>(Position& pos, Move* moveList);
template Move* generate<QUIET>(Position& pos, Move* moveList);
template Move* generate<CAPTURES>(Position& pos, Move* moveList);


template <>
Move* generate<LEGAL>(Position& pos, Move* moveList) {

    Move pseudoLegalMoves[MAX_MOVES];
    Move* end = pos.checkers() ? generate<EVASIONS>(pos, pseudoLegalMoves) 
                               : generate<NON_EVASIONS>(pos, pseudoLegalMoves);

    for (Move* m = pseudoLegalMoves; m < end; ++m) {
        if (pos.is_legal(*m))
            *moveList++ = *m;
    }

    return moveList;
}


} // namespace harukashogi