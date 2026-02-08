#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "position.h"

namespace harukashogi {


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


constexpr size_t MAX_MOVES = 593;

} // namespace harukashogi

#endif