#ifndef PERFT_H
#define PERFT_H

#include "position.h"

namespace harukashogi {


// performs a perft test at a given depth
// returns the number of leaves visited
int perft(Position& pos, int depth);

// performs a perft test at a given depth
// prints the results perft results for each depth
// also prints results for each move ot the max depth
void perft_test(Position& pos, int depth);


} // namespace harukashogi

#endif // PERFT_H