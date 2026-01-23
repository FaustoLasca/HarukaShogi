#include <iostream>

#include "movegen.h"
#include "types.h"
#include "position.h"

namespace harukashogi {


constexpr bool dir_in_bounds(Square square, Direction d) {
    int d_file = file_of(square) + d % 9;
    int d_rank = rank_of(square) + d / 9;
    return d_file >= F_1 && d_file <= F_9 && d_rank >= R_1 && d_rank <= R_9;
}


bool is_attacked(Position& pos, Square square, Color by) {
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

        // std::cout << "PieceType: " << int(pt) << std::endl;
        // std::cout << "Piece: " << int(p) << std::endl;

        for (int i = 0; i < NUM_DIRECTIONS; ++i) {
            d = colorFactor * StandardMoveDirections[pt * NUM_DIRECTIONS + i];
            to = add_direction(square, d);
            if (to != NO_SQUARE)
                if (pos.piece(to) == p )
                    return true;
        }

        // check sliding moves if necessary
        if (pt == BISHOP || pt == ROOK) {
            for (int i = 0; i < MAX_SLIDING_DIRECTIONS; ++i) {
                // sliding moves are symmetric, so we can use the same direction for both colors
                d = SlidingMoveDirections[sliding_type_index(pt) * MAX_SLIDING_DIRECTIONS + i];
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


} // namespace harukashogi