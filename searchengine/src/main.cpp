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
    TTable* tt = new TTable();

    std::cout << sizeof(*tt) << std::endl;

    uint64_t key = 0x1234567890abcdef;
    std::cout << "key: " << key << std::endl;
    std::cout << "idx: " << tt->index(key) << std::endl;
    std::tuple<bool, TTEntry*> result = tt->probe(key);
    bool hit = std::get<0>(result);
    TTEntry* entry = std::get<1>(result);

    std::cout << "hit: " << hit << std::endl;
    std::cout << "entry: " << entry->key << std::endl;

    return 0;
}