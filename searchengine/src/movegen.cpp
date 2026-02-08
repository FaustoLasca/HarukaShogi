#include "movegen.h"
#include "types.h"
#include "position.h"
#include "bitboard.h"
#include "misc.h"

namespace harukashogi {


template<Color c, Direction d>
Move* splat_dir_moves(Position& pos, Move* moveList, Bitboard attacks) {
    while (attacks) {
        Square to = pop_lsb(attacks);
        Square from = to - dir_delta(d);
        if (can_promote(type_of(pos.piece(from))))
            if (promotion_zone(to, c) || promotion_zone(from, c))
                *moveList++ = Move(from, to, true);
        *moveList++ = Move(from, to);
    }

    return moveList;
}


template<Color c, PieceType pt>
Move* splat_piece_Moves(Position& pos, Move* moveList, Bitboard attacks, Square from) {
    while (attacks) {
        Square to = pop_lsb(attacks);
        if constexpr (can_promote(pt))
            if (promotion_zone(to, c) || promotion_zone(from, c))
                *moveList++ = Move(from, to, true);
        *moveList++ = Move(from, to);
    }

    return moveList;
}


// generates pseudo-legal moves for a given color and direction
// (non king and non sliding pieces)
template<Color c, Direction d>
Move* generate(Position& pos, Move* moveList, Bitboard target) {
    Bitboard attacks = dir_attacks_bb<d>(pos.dir_pieces(c, d));
    attacks &= target;
    return splat_dir_moves<c, d>(pos, moveList, attacks);
}




// generates pseudo-legal direction moves of the given type for a given color
// (non king and non sliding pieces)
template<Color c>
Move* generate_all_direction(Position& pos, Move* moveList, Bitboard target) {
    moveList = generate<c, N_DIR>(pos, moveList, target);
    moveList = generate<c, NE_DIR>(pos, moveList, target);
    moveList = generate<c, E_DIR>(pos, moveList, target);
    moveList = generate<c, SE_DIR>(pos, moveList, target);
    moveList = generate<c, S_DIR>(pos, moveList, target);
    moveList = generate<c, SW_DIR>(pos, moveList, target);
    moveList = generate<c, W_DIR>(pos, moveList, target);
    moveList = generate<c, NW_DIR>(pos, moveList, target);
    if constexpr (c == BLACK) {
        moveList = generate<c, NNE_DIR>(pos, moveList, target);
        moveList = generate<c, NNW_DIR>(pos, moveList, target);
    }
    else {
        moveList = generate<c, SSE_DIR>(pos, moveList, target);
        moveList = generate<c, SSW_DIR>(pos, moveList, target);
    }

    return moveList;
}




template <Color c, PieceType pt>
Move* generate_sliding(Position& pos, Move* moveList, Bitboard target) {
    Bitboard bb = pos.sld_pieces(c, pt);
    Square from;
    Bitboard attacks;

    while (bb) {
        from = pop_lsb(bb);
        attacks = sld_attacks_bb<c, pt>(from, pos.all_pieces());
        attacks &= target;

        moveList = splat_piece_Moves<c, pt>(pos, moveList, attacks, from);
    }

    return moveList;
}


template<Color c>
Move* generate_all_sliding(Position& pos, Move* moveList, Bitboard target) {
    moveList = generate_sliding<c, LANCE>(pos, moveList, target);
    moveList = generate_sliding<c, BISHOP>(pos, moveList, target);
    moveList = generate_sliding<c, P_BISHOP>(pos, moveList, target);
    moveList = generate_sliding<c, ROOK>(pos, moveList, target);
    moveList = generate_sliding<c, P_ROOK>(pos, moveList, target);

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
        moveList = add_drop<c, SILVER>(pos, moveList, sq);
        moveList = add_drop<c, LANCE>(pos, moveList, sq);
        moveList = add_drop<c, KNIGHT>(pos, moveList, sq);
        moveList = add_drop<c, BISHOP>(pos, moveList, sq);
        moveList = add_drop<c, ROOK>(pos, moveList, sq);
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

        moveList = generate_all_sliding<c>(pos, moveList, target);
        moveList = generate_all_direction<c>(pos, moveList, target);

        if constexpr (gt != CAPTURES) {
            moveList = generate_drops<c>(pos, moveList, target);
        }

    }

    // treat king moves separately, as the logic is different for EVASION
    Bitboard kingBb = dir_attacks_bb<c, KING>(pos.king_square(c));
    kingBb &= gt == EVASIONS ? ~pos.all_pieces(c) : target; 
    moveList = splat_piece_Moves<c, KING>(pos, moveList, kingBb, pos.king_square(c));

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