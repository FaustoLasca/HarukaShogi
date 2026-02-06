#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "position.h"

namespace harukashogi {


// generate all legal moves
// moves are added to the move list and the first free slot is returned
Move* legacy_generate_moves(Position& pos, Move* moveList);

// generate all legal moves from the piece in the given square
// moves are added to the move list and the first free slot is returned
Move* piece_moves(Position& pos, Move* moveList, Square from);

// returns the squares a piece attacks
bool is_attacked(const Position& pos, Square square, Color by);


// initializes the precomputed data structures for move generation
void init_movegen();


enum GenType {
    EVASIONS,
    NON_EVASIONS,
    QUIET,
    CAPTURES,

    LEGAL
};


template <GenType gt>
Move* generate(Position& pos, Move* moveList);

// data structures for move generation
// directions each piece type can move to (black perspective)
// sliding directions are excluded
constexpr std::array<DirectionStruct, NUM_PIECE_TYPES * NUM_1DIR> StandardMoveDirections = {
    NORTH_EAST, NORTH,  NORTH_WEST, WEST,   SOUTH_WEST, SOUTH,  SOUTH_EAST, EAST, // KING
    NORTH_EAST, NORTH, NORTH_WEST, WEST,    SOUTH,  EAST,   NO_DIR, NO_DIR, // GOLD
    NORTH_EAST, NORTH, NORTH_WEST, SOUTH_WEST, SOUTH_EAST, NO_DIR, NO_DIR, NO_DIR, // SILVER
    NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, // LANCE
    2*NORTH + EAST, 2*NORTH + WEST, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, // KNIGHT
    NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, // BISHOP
    NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, // ROOK
    NORTH, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, NO_DIR, // PAWN

    NORTH_EAST, NORTH, NORTH_WEST, WEST,    SOUTH,  EAST,   NO_DIR, NO_DIR, // P_SILVER
    NORTH_EAST, NORTH, NORTH_WEST, WEST,    SOUTH,  EAST,   NO_DIR, NO_DIR, // P_LANCE
    NORTH_EAST, NORTH, NORTH_WEST, WEST,    SOUTH,  EAST,   NO_DIR, NO_DIR, // P_KNIGHT
    NORTH, WEST,    SOUTH,  EAST,   NO_DIR, NO_DIR, NO_DIR, NO_DIR, // P_BISHOP
    NORTH_EAST, NORTH_WEST, SOUTH_WEST, SOUTH_EAST, NO_DIR, NO_DIR, NO_DIR, NO_DIR, // P_ROOK
    NORTH_EAST, NORTH, NORTH_WEST, WEST,    SOUTH,  EAST,   NO_DIR, NO_DIR, // P_PAWN
};


// separate data structure for sliding directions
// indexed by sliding_type_index(pt) * MAX_SLIDING_DIRECTIONS
constexpr std::array<DirectionStruct, NUM_SLIDING_TYPES * MAX_SLIDING_DIRECTIONS> SlidingMoveDirections = {
    NORTH_EAST, NORTH_WEST, SOUTH_WEST, SOUTH_EAST, // BISHOP
    NORTH_EAST, NORTH_WEST, SOUTH_WEST, SOUTH_EAST, // P_BISHOP (same as BISHOP)
    NORTH, WEST,    SOUTH,  EAST,   // ROOK
    NORTH, WEST,    SOUTH,  EAST,   // P_ROOK (same as ROOK)
    NORTH, NO_DIR, NO_DIR, NO_DIR // LANCE
};


constexpr size_t MAX_MOVES = 593;

} // namespace harukashogi

#endif