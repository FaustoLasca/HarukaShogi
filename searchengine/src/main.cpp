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

using namespace harukashogi;


int main() {
    init();

    Searcher searcher(false);
    searcher.set_position();

    std::cout << searcher.search(600000, 6) << std::endl;

    return 0;
}