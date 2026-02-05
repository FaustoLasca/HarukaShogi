#include "movegen.h"
#include "types.h"
#include "position.h"
#include "bitboard.h"

namespace harukashogi {


bool is_attacked(const Position& pos, Square square, Color by) {
    // -1 for black, 1 for white
    // this is used to invert the direction of the move,
    // since we look starting from the destination square
    int colorFactor = (by == BLACK) ? -1 : 1;
    DirectionStruct d;
    Square to = square;
    Piece p;

    // for every piece type, generate inverse move and check if piece is there
    for (PieceType pt = KING; pt < NUM_PIECE_TYPES; ++pt) {
        p = make_piece(by, pt);

        for (int i = 0; i < NUM_1DIR; ++i) {
            d = colorFactor * StandardMoveDirections[pt * NUM_1DIR + i];
            if (d == NO_DIR)
                break;

            to = add_direction(square, d);
            if (to != NO_SQUARE)
                if (pos.piece(to) == p )
                    return true;
        }

        // check sliding moves if necessary
        if (sliding_type_index(pt) != -1) {
            for (int i = 0; i < MAX_SLIDING_DIRECTIONS; ++i) {
                d = colorFactor * SlidingMoveDirections[
                    sliding_type_index(pt) * MAX_SLIDING_DIRECTIONS + i];
                if (d == NO_DIR)
                    break;

                to = add_direction(square, d);
                // loop until out of bounds or piece is found
                while (to != NO_SQUARE) {
                    if (pos.piece(to) == p)
                        return true;
                    // stop if we hit a piece
                    else if (pos.piece(to) != NO_PIECE)
                        break;

                    to = add_direction(to, d);
                }
            }
        }
    }

    return false;
}


Move* piece_moves(Position& pos, Move* moveList, Square from) {
    PieceType pt = type_of(pos.piece(from));
    PieceType captured = NO_PIECE_TYPE;
    Move move = Move::null();
    Color color = color_of(pos.piece(from));
    int colorFactor = (color == BLACK) ? 1 : -1;
    bool promotion = false;
    bool forced_promotion = false;
    Rank lastRank = (color == BLACK) ? R_1 : R_9;
    Rank secondLastRank = (color == BLACK) ? R_2 : R_8;
    DirectionStruct d;
    Square to = from;

    // standard moves
    for (int i = 0; i < NUM_1DIR; ++i) {
        d = colorFactor * StandardMoveDirections[pt * NUM_1DIR + i];
        if (d == NO_DIR)
            break;

        to = add_direction(from, d);
        if (to != NO_SQUARE) {
            // empty square or capture
            if (pos.piece(to) == NO_PIECE || color_of(pos.piece(to)) == ~color)
                move = Move(from, to);

            // check if promotion is available
            if ((promotion_zone(to, color) || promotion_zone(from, color))
                    && !is_promoted(pt) && can_promote(pt)) {
                promotion = true;
                // check if promotion is forced
                // applies to pawns and knights for standard moves
                if ((pt == PAWN || pt == KNIGHT) && rank_of(move.to()) == lastRank)
                    forced_promotion = true;
                else if (pt == KNIGHT && rank_of(move.to()) == secondLastRank)
                    forced_promotion = true;
            }
           

            // add move to the list
            if (!move.is_null()) {
                if (pos.is_legal(move)) {
                    if (!forced_promotion)
                        *moveList++ = move;
                    if (promotion) {
                        move = Move(move.from(), move.to(), true);
                        *moveList++ = move;
                    }
                }
            }

            // reset flags and move
            promotion = false;
            forced_promotion = false;
            move = Move::null();
            captured = NO_PIECE_TYPE;
        }
    }

    // sliding moves
    if (sliding_type_index(pt) != -1) {
        bool collision = false;

        for (int i = 0; i < MAX_SLIDING_DIRECTIONS; ++i) {
            d = colorFactor * SlidingMoveDirections[sliding_type_index(pt) * MAX_SLIDING_DIRECTIONS + i];
            if (d == NO_DIR)
                break;

            to = add_direction(from, d);

            collision = false;

            while (to != NO_SQUARE && !collision) {
                if (pos.piece(to) == NO_PIECE)
                    move = Move(from, to);

                else if (color_of(pos.piece(to)) == ~color) {
                    move = Move(from, to);
                    collision = true;
                }

                else {
                    collision = true;
                }

                // check if promotion is available
                if ((promotion_zone(to,color) || promotion_zone(from, color)) && !is_promoted(pt)) {
                    promotion = true;
                    // check if promotion is forced
                    // only applies to lance for sliding moves
                    if (pt == LANCE && rank_of(move.to()) == lastRank)
                        forced_promotion = true;
                }

                // if valid move, add to move list
                if (!move.is_null()) {
                    if (pos.is_legal(move)) {
                        if (!forced_promotion)
                            *moveList++ = move;
                        if (promotion) {
                            move = Move(move.from(), move.to(), true);
                            *moveList++ = move;
                        }
                    }
                }

                // reset flags and move
                promotion = false;
                forced_promotion = false;
                move = Move::null();
                captured = NO_PIECE_TYPE;

                to = add_direction(to, d);
            }
        }
    }

    return moveList;
}


// generates all drop from the given position
// the moves are added to the move list and the first free slot is returned
Move* drop_moves(Position& pos, Move* moveList) {
    Color toMove = pos.side_to_move();
    int colorFactor = (toMove == BLACK) ? 1 : -1;
    Square pawnAttack;
    Move move = Move::null();
    Rank lastRanks[2];
    lastRanks[0] = (toMove == BLACK) ? R_1 : R_9;
    lastRanks[1] = (toMove == BLACK) ? R_2 : R_8;

    // loop through all free squares
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (pos.piece(sq) == NO_PIECE) {

            // loop through the pieces in the hand
            // the pawn is special, so we handle it separately
            for (PieceType pt = GOLD; pt < PAWN; ++pt) {
                if (pos.hand_count(toMove, pt) > 0) {

                    // LANCE and KNIGHT cannot be dropped on the last rank
                    if ((pt == KNIGHT || pt == LANCE) && rank_of(sq) == lastRanks[0])
                        continue;
                    // KNIGHT cannot be dropped on the second last rank either
                    else if (pt == KNIGHT && rank_of(sq) == lastRanks[1])
                        continue;

                    // add the drop to the move list
                    move = Move(pt, sq);
                    // TODO: no need to always check if the move is legal, needs optimization
                    if (pos.is_legal(move))
                        *moveList++ = move;
                    
                }
            }

            // handle the pawn separately
            // the pawn can't be dropped on the last rank
            // the pawn can't be dropped on the same rank as other pawns
            // the pawn drop can't checkmate (this is checked in is_legal)
            if (pos.hand_count(toMove, PAWN) > 0) {
                if (!pos.pawn_on_file(toMove, file_of(sq)) && rank_of(sq) != lastRanks[0]) {
                    move = Move(PAWN, sq);

                    if (pos.is_legal(move))
                        *moveList++ = move;
                }
            }
        }
    }

    return moveList;
}


Move* legacy_generate_moves(Position& pos, Move* moveList) {
    // generate drop moves
    moveList = drop_moves(pos, moveList);

    // for each piece on the board, generate piece moves
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (pos.piece(sq) != NO_PIECE && color_of(pos.piece(sq)) == pos.side_to_move())
            moveList = piece_moves(pos, moveList, sq);
    }

    return moveList;
}


// generates pseudo-legal moves for a given color and direction
// (non king and non sliding pieces)
template<Color c, Direction d>
Move* generate(Position& pos, Move* moveList, Bitboard target) {
    Bitboard attacks = dir_attacks_bb<d>(pos.dir_pieces(c, d));
    attacks &= target;

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


// generates pseudo-legal direction moves of the given type for a given color
// (non king and non sliding pieces)
template<GenType gt, Color c>
Move* generate_all_direction(Position& pos, Move* moveList) {
    // TODO: missing EVASIONS implementation yet
    Bitboard target = gt == NON_EVASIONS ? ~pos.all_pieces(c)
                    : gt == CAPTURES     ? pos.all_pieces(~c)
                                         : ~pos.all_pieces(); // QUIETS

    
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

// FOR TESTING
template Move* generate_all_direction<QUIET, BLACK>(Position& pos, Move* moveList);
template Move* generate_all_direction<CAPTURES, BLACK>(Position& pos, Move* moveList);
template Move* generate_all_direction<NON_EVASIONS, BLACK>(Position& pos, Move* moveList);
template Move* generate_all_direction<QUIET, WHITE>(Position& pos, Move* moveList);
template Move* generate_all_direction<CAPTURES, WHITE>(Position& pos, Move* moveList);
template Move* generate_all_direction<NON_EVASIONS, WHITE>(Position& pos, Move* moveList);


template<Color c, PieceType pt>
Bitboard sliding_attacks_bb(Square from, Bitboard occupied) {
    Bitboard bb;
    Direction d;
    Bitboard attacks = 0;

    constexpr size_t index = sl_dir_index(make_piece(c, pt));

    for (size_t i=0; i<4 && PSlidingDirections[index][i] != NULL_DIR; ++i) {
        d = PSlidingDirections[index][i];
        bb = square_bb(from);

        while (bb) {
            // TODO: THIS IS HORRIFYING, CHECK FOR A BETTER COMPILE TIME SOLUTION
            switch (d) {
                case N_DIR:
                    bb = dir_attacks_bb<N_DIR>(bb);
                    break;
                case NE_DIR:
                    bb = dir_attacks_bb<NE_DIR>(bb);
                    break;
                case E_DIR:
                    bb = dir_attacks_bb<E_DIR>(bb);
                    break;
                case SE_DIR:
                    bb = dir_attacks_bb<SE_DIR>(bb);
                    break;
                case S_DIR:
                    bb = dir_attacks_bb<S_DIR>(bb);
                    break;
                case SW_DIR:
                    bb = dir_attacks_bb<SW_DIR>(bb);
                    break;
                case W_DIR:
                    bb = dir_attacks_bb<W_DIR>(bb);
                    break;
                case NW_DIR:
                    bb = dir_attacks_bb<NW_DIR>(bb);
                default:
                    break;
            }
            
            attacks |= bb;
            bb &= ~occupied;
        }
    }

    return attacks;
}


template <Color c, PieceType pt>
Move* generate_sliding(Position& pos, Move* moveList, Bitboard target) {
    Bitboard bb = pos.sld_pieces(c, pt);
    Square from, to;
    Bitboard attacks;

    while (bb) {
        from = pop_lsb(bb);
        attacks = sliding_attacks_bb<c, pt>(from, pos.all_pieces());
        attacks &= target;

        while (attacks) {
            to = pop_lsb(attacks);
            if (promotion_zone(to, c) || promotion_zone(from, c))
                *moveList++ = Move(from, to, true);
            *moveList++ = Move(from, to);
        }
    }

    return moveList;
}


template Move* generate_sliding<BLACK, LANCE>(Position& pos, Move* moveList, Bitboard target);
template Move* generate_sliding<WHITE, LANCE>(Position& pos, Move* moveList, Bitboard target);
template Move* generate_sliding<BLACK, BISHOP>(Position& pos, Move* moveList, Bitboard target);
template Move* generate_sliding<WHITE, BISHOP>(Position& pos, Move* moveList, Bitboard target);
template Move* generate_sliding<BLACK, ROOK>(Position& pos, Move* moveList, Bitboard target);
template Move* generate_sliding<WHITE, ROOK>(Position& pos, Move* moveList, Bitboard target);


} // namespace harukashogi