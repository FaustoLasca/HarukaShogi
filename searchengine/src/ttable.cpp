#include <iostream>
#include <atomic>

#include "ttable.h"

namespace harukashogi {


constexpr uint8_t DEPTH_BITS = 6;
constexpr uint8_t DEPTH_MASK = (1ull << DEPTH_BITS) - 1;

constexpr uint8_t NUM_GENERATIONS = 255;

// an entry in the transposition table
// 80 bits, 10 bytes in total
struct TTEntry {
    std::atomic<uint16_t> key_low;
    std::atomic<uint16_t> key_high;
    std::atomic<int16_t> score;
    std::atomic<Move> bestMove;
    // the first 6 bits are for the depth
    // the last 2 bits are for the node type
    std::atomic<uint8_t> depthAndNodeType;
    // the generation is a number between 1 and 255
    // this is because 0 is reserved for empty entries
    std::atomic<uint8_t> generation8;

    TTData read() const {
        return TTData(score, bestMove, depthAndNodeType & DEPTH_MASK,
                      NodeType(depthAndNodeType >> DEPTH_BITS));
    }

    void write(uint64_t key, int16_t score, Move bestMove, uint8_t depth, NodeType type,
               int generation) {
        this->key_low = uint16_t(key);
        this->key_high = uint16_t(key >> 16);
        this->score = score;
        this->bestMove = bestMove;
        this->depthAndNodeType = (depth & DEPTH_MASK) | (uint8_t(type) << DEPTH_BITS);
        this->generation8 = generation % NUM_GENERATIONS + 1;
    }

    bool matches(uint64_t key) const {
        return key_low == uint16_t(key) && key_high == uint16_t(key >> 16);
    }

    bool is_empty() const {
        return !depthAndNodeType;
    }
};


constexpr size_t CLUSTER_SIZE = 3;

// 32 bytes for each cluster
// (efficiently aligned for cache lines)
struct Cluster {
    TTEntry entries[CLUSTER_SIZE];
    char padding[32 - sizeof(TTEntry) * CLUSTER_SIZE];
};


void TTWriter::write(uint64_t key, int16_t score, Move bestMove, uint8_t depth, NodeType type) {
    entry->write(key, score, bestMove, depth, type, gen8);
}


// resize the transposition table to the given size in MB
void TTable::resize(size_t size) {
    size_t numClusters = size * 1024 * 1024 / sizeof(Cluster);
    table = std::make_unique<Cluster[]>(numClusters);
    this->size = numClusters;
}


TTable::TTable() {
    resize(16);
}

TTable::~TTable() = default;


uint8_t TTable::relativeAge(uint8_t gen) const {
    return gen > generation8 ? gen - NUM_GENERATIONS : gen;
}

std::tuple<bool, TTData, TTWriter> TTable::probe(uint64_t key) {
    TTEntry* entries = table[index(key)].entries;

    // loop through the cluster and 
    for (size_t i = 0; i < CLUSTER_SIZE; i++) {
        if (entries[i].matches(key) && !entries[i].is_empty()) {
            hits++;
            return {true, entries[i].read(), TTWriter(&entries[i], generation8)};
        }
    }

    // if no matching entry is found, decide which entry to replace and return it's pointer
    // return the pointer to the oldest entry
    TTEntry* replace = entries;
    for (size_t i = 0; i < CLUSTER_SIZE; i++) {
        // return the first empty entry
        if (entries[i].is_empty())
            return {false, TTData(), TTWriter(&entries[i], generation8)};
        if (relativeAge(entries[i].generation8) < relativeAge(replace->generation8))
            replace = &entries[i];
    }
    
    collisions++;
    return {false, TTData(), TTWriter(replace, generation8)};
}


void TTable::new_search() {
    generation8++;
    if (generation8 == 0)
        generation8 = 1;
}


void TTable::print_stats() const {
    int count = 0;
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < CLUSTER_SIZE; j++) {
            if (!table[i].entries[j].is_empty())
                count++;
        }
    }
    
    std::cout << "TT Size:    " << size * CLUSTER_SIZE << std::endl;
    std::cout << "Occupied:   " << count << std::endl;
    std::cout << "Hits:       " << hits << std::endl;
    std::cout << "Collisions: " << collisions << std::endl;
}


// fast modulo hashing using multiplication and bit shift
// returns a number in the range [0, ttSize)
size_t TTable::index(uint64_t key) const {
    return (__uint128_t(key) * __uint128_t(size)) >> 64;
}


} // namespace harukashogi