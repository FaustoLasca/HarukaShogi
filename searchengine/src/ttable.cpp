#include <iostream>

#include "ttable.h"

namespace harukashogi {


std::tuple<bool, TTEntry*> TTable::probe(uint64_t key) {
    size_t idx = index(key);
    TTEntry* entry = &table[idx];
    bool hit = false;

    // hit if the same key is found
    if (entry->key == key) {
        hit = true;
        ++hits;
    // collision if another key is found
    } else if (entry->key != 0) {
        ++collisions;
    // new entry if no key is found
    } else {
        ++count;
    }

    return std::make_tuple(hit, entry);
}


void TTable::print_stats() const {
    std::cout << "Count: " << count << std::endl;
    std::cout << "Hits: " << hits << std::endl;
    std::cout << "Collisions: " << collisions << std::endl;
}


} // namespace harukashogi