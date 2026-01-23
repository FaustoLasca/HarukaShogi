#include <iostream>

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
                // sliding moves are symmetric, so we can use the same direction for both colors
                d = SlidingMoveDirections[sliding_type_index(pt) * MAX_SLIDING_DIRECTIONS + i];
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

            if (!move.is_null()) {
                if (pos.is_legal(move)) {
                    *moveList++ = move;
                    if (promotion_zone(to, color)) {
                        move.promotion = true;
                        *moveList++ = move;
                    }
                }
                move = NULL_MOVE;
                captured = NO_PIECE_TYPE;
            }
        }
    }

    // sliding moves
    if (sliding_type_index(pt) != -1) {
        bool collision = false;

        for (int i = 0; i < MAX_SLIDING_DIRECTIONS; ++i) {
            d = SlidingMoveDirections[sliding_type_index(pt) * MAX_SLIDING_DIRECTIONS + i];
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

                // if valid move, add to move list
                if (!move.is_null()) {
                    if (pos.is_legal(move)) {
                        *moveList++ = move;
                        if (promotion_zone(to, color)) {
                            move.promotion = true;
                            *moveList++ = move;
                        }
                    }
                    move = NULL_MOVE;
                    captured = NO_PIECE_TYPE;
                }

                to = add_direction(to, d);
            }
        }
    }

    return moveList;
}


} // namespace harukashogi