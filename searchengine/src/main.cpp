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
#include "transposition.h"

using namespace harukashogi;


int main() {
    
    TTEntry entry = TTEntry();

    std::cout << sizeof(entry) << std::endl;

    TranspositionTable tt;

    std::cout << sizeof(tt) << std::endl;

    return 0;
}