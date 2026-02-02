#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "position.h"

namespace harukashogi {


// generate all legal moves
// moves are added to the move list and the first free slot is returned
Move* generate_moves(Position& pos, Move* moveList);

// generate all legal moves from the piece in the given square
// moves are added to the move list and the first free slot is returned
Move* piece_moves(Position& pos, Move* moveList, Square from);

// returns the squares a piece attacks
bool is_attacked(const Position& pos, Square square, Color by);


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

constexpr Direction PTDirections[NUM_PIECES][8] = {
    // black pieces
    {N_DIR,    NE_DIR,   E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NW_DIR  }, // KING
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // GOLD
    {N_DIR,    NE_DIR,   SE_DIR,   SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR}, // SILVER
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // LANCE
    {NNE_DIR,  NNW_DIR,  NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // KNIGHT
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // BISHOP
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // ROOK
    {N_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // PAWN
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_SILVER
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_LANCE
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_KNIGHT
    {N_DIR,    E_DIR,    S_DIR,    W_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_BISHOP
    {NE_DIR,   SE_DIR,   SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_ROOK
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_PAWN

    // white pieces
    {N_DIR,    NE_DIR,   E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NW_DIR  }, // KING
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // GOLD
    {NE_DIR,   SE_DIR,   S_DIR,    SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR}, // SILVER
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // LANCE
    {SSE_DIR,  SSW_DIR,  NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // KNIGHT
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // BISHOP
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // ROOK
    {S_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // PAWN
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_SILVER
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_LANCE
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_KNIGHT
    {N_DIR,    E_DIR,    S_DIR,    W_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_BISHOP
    {NE_DIR,   SE_DIR,   SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_ROOK
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_PAWN
};

// separate data structure for sliding directions
constexpr std::array<DirectionStruct, NUM_SLIDING_TYPES * MAX_SLIDING_DIRECTIONS> SlidingMoveDirections = {
    NORTH_EAST, NORTH_WEST, SOUTH_WEST, SOUTH_EAST, // BISHOP
    NORTH, WEST,    SOUTH,  EAST,   // ROOK
    NORTH, NO_DIR, NO_DIR, NO_DIR // LANCE
};

constexpr size_t MAX_MOVES = 593;

} // namespace harukashogi

#endif