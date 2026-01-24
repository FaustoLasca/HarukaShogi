#include "movegen.h"
#include "types.h"
#include "position.h"

namespace harukashogi {


bool is_attacked(const Position& pos, Square square, Color by) {
    // -1 for black, 1 for white
    // this is used to invert the direction of the move,
    // since we look starting from the destination square
    int colorFactor = (by == BLACK) ? -1 : 1;
    Direction d;
    Square to = square;
    Piece p;

    // for every piece type, generate inverse move and check if piece is there
    for (PieceType pt = KING; pt < NUM_PIECE_TYPES; ++pt) {
        p = make_piece(by, pt);

        for (int i = 0; i < NUM_DIRECTIONS; ++i) {
            d = colorFactor * StandardMoveDirections[pt * NUM_DIRECTIONS + i];
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
                d = colorFactor * SlidingMoveDirections[sliding_type_index(pt) * MAX_SLIDING_DIRECTIONS + i];
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
    Move move = NULL_MOVE;
    Color color = color_of(pos.piece(from));
    int colorFactor = (color == BLACK) ? 1 : -1;
    bool promotion = false;
    bool forced_promotion = false;
    Rank lastRank = (color == BLACK) ? R_1 : R_9;
    Rank secondLastRank = (color == BLACK) ? R_2 : R_8;
    Direction d;
    Square to = from;

    // standard moves
    for (int i = 0; i < NUM_DIRECTIONS; ++i) {
        d = colorFactor * StandardMoveDirections[pt * NUM_DIRECTIONS + i];
        if (d == NO_DIR)
            break;

        to = add_direction(from, d);
        if (to != NO_SQUARE) {
            // empty square
            if (pos.piece(to) == NO_PIECE)
                move = Move{from, to, false, NO_PIECE_TYPE};

            // capture piece
            else if (color_of(pos.piece(to)) == ~color)
                move = Move{from, to, false, type_of(pos.piece(to))};

            // check if promotion is available
            if ((promotion_zone(to, color) || promotion_zone(from, color)) && !is_promoted(pt) && can_promote(pt)) {
                promotion = true;
                // check if promotion is forced
                // applies to pawns and knights for standard moves
                if ((pt == PAWN || pt == KNIGHT) && rank_of(move.to) == lastRank)
                    forced_promotion = true;
                else if (pt == KNIGHT && rank_of(move.to) == secondLastRank)
                    forced_promotion = true;
            }
           

            // add move to the list
            if (!move.is_null()) {
                if (pos.is_legal(move)) {
                    if (!forced_promotion)
                        *moveList++ = move;
                    if (promotion) {
                        move.promotion = true;
                        *moveList++ = move;
                    }
                }
            }

            // reset flags and move
            promotion = false;
            forced_promotion = false;
            move = NULL_MOVE;
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
                    move = Move{from, to, false, NO_PIECE_TYPE};

                else if (color_of(pos.piece(to)) == ~color) {
                    move = Move{from, to, false, type_of(pos.piece(to))};
                    collision = true;
                }

                else {
                    collision = true;
                }

                // check if promotion is available
                if ((promotion_zone(to, color) || promotion_zone(from, color)) && !is_promoted(pt)) {
                    promotion = true;
                    // check if promotion is forced
                    // only applies to lance for sliding moves
                    if (pt == LANCE && rank_of(move.to) == lastRank)
                        forced_promotion = true;
                }

                // if valid move, add to move list
                if (!move.is_null()) {
                    if (pos.is_legal(move)) {
                        if (!forced_promotion)
                            *moveList++ = move;
                        if (promotion) {
                            move.promotion = true;
                            *moveList++ = move;
                        }
                    }
                }

                // reset flags and move
                promotion = false;
                forced_promotion = false;
                move = NULL_MOVE;
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
    Move move = NULL_MOVE;
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
                    move = Move{NO_SQUARE, sq, false, pt};
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
                    move = Move{NO_SQUARE, sq, false, PAWN};

                    if (pos.is_legal(move))
                        *moveList++ = move;
                }
            }
        }
    }

    return moveList;
}


Move* generate_moves(Position& pos, Move* moveList) {
    // generate drop moves
    moveList = drop_moves(pos, moveList);

    // for each piece on the board, generate piece moves
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if (pos.piece(sq) != NO_PIECE && color_of(pos.piece(sq)) == pos.side_to_move())
            moveList = piece_moves(pos, moveList, sq);
    }

    return moveList;
}


} // namespace harukashogi