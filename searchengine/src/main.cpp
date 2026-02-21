#include <iostream>
#include <immintrin.h>
#include <bitset>
#include <chrono>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"
#include "movegen.h"
#include "search.h"
#include "movepicker.h"
#include "history.h"
#include "thread.h"

using namespace harukashogi;


int main() {
    init();

    Searcher searcher;
    searcher.set_position();

    Move move = searcher.search(1000, 20);

    std::cout << "Best move:  " << move << std::endl;

    searcher.print_stats();

    return 0;
}