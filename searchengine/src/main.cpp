#include <iostream>
#include <random>
#include <iomanip>
#include <bitset>

#include "movegen.h"
#include "types.h"
#include "position.h"
#include "search.h"
#include "misc.h"
#include "perft.h"
#include "ttable.h"

using namespace harukashogi;


int main() {
    
    init();

    Searcher searcher;
    searcher.set_position();

    Move bestMove = searcher.search(chr::milliseconds(600000), 4);

    std::cout << "Best move: " << bestMove << std::endl;

    searcher.print_stats();

    return 0;
}