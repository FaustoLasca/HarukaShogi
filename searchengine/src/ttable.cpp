#include <iostream>

#include "ttable.h"

namespace harukashogi {

constexpr size_t CLUSTER_SIZE = 4;

// 32 bytes for each cluster
// (efficiently aligned for cache lines)
struct Cluster {
    TTEntry entries[CLUSTER_SIZE];
};


// 100 MB transposition table size
constexpr size_t TT_MB_SIZE = 100;
constexpr size_t TT_SIZE = TT_MB_SIZE * 1024 * 1024 / sizeof(Cluster);


TTable::TTable() {
    table = std::make_unique<Cluster[]>(TT_SIZE);
}

TTable::~TTable() = default;

std::tuple<bool, TTEntry*> TTable::probe(uint64_t key) {
    TTEntry* entries = table[index(key, TT_SIZE)].entries;
    uint16_t key16 = uint16_t(key);

    // loop through the cluster and 
    for (size_t i = 0; i < CLUSTER_SIZE; i++) {
        if (entries[i].key == key16) {
            hits++;
            return std::make_tuple(entries[i].bestMove != Move::null(), &entries[i]);
        }
    }

    // if no matching entry is found, decide which entry to replace and return it's pointer
    // return the pointer to the entry with the lowest depth
    TTEntry* minDepthEntry = entries;
    for (size_t i = 0; i < CLUSTER_SIZE; i++) {
        if (entries[i].depth < minDepthEntry->depth)
            minDepthEntry = &entries[i];
    }

    // if the entry is not empty, it is a collision
    if (minDepthEntry->bestMove != Move::null())
        collisions++;
    else
        count++;

    return std::make_tuple(false, minDepthEntry);
}


void TTable::print_stats() const {
    std::cout << "TT Size:    " << TT_SIZE * CLUSTER_SIZE << std::endl;
    std::cout << "Occupied:   " << count << std::endl;
    std::cout << "Hits:       " << hits << std::endl;
    std::cout << "Collisions: " << collisions << std::endl;
}


// fast modulo hashing using multiplication and bit shift
// returns a number in the range [0, ttSize)
size_t TTable::index(uint64_t key, uint64_t ttSize) const {
    return (__uint128_t(key) * __uint128_t(ttSize)) >> 64;
}


} // namespace harukashogi