#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include <memory>

#include "types.h"

namespace harukashogi {


// node type for the transposition table
// needed to know how to use the entry
// PV_NODE: (eval between alpha and beta) exact score
// CUT_NODE: (eval higher than beta, pruned) lowerbound score
// ALL_NODE: (eval lower than alpha) upperbound score
//          (children could have been pruned)
enum TTEntryType : uint8_t {
    PV_ENTRY,
    CUT_ENTRY,
    ALL_ENTRY
};


// data of a TTEntry. When the TT is probed, a copy of this data is returned.
// particularly useful for parallel search. When probing the TT, a copy of the data is returned.
// This helps avoid race conditions when updating the TT.
struct TTData {
    int16_t score = 0;
    Move bestMove = Move::null();
    uint8_t depth = 0;
    TTEntryType type = ALL_ENTRY;

    TTData() = default;
    TTData(int16_t score, Move bestMove, uint8_t depth, TTEntryType type) :
        score(score), bestMove(bestMove), depth(depth), type(type) {}
};


struct Cluster;
struct TTEntry;


class TTWriter {
    public:
        void write(uint64_t key, int16_t score, Move bestMove, uint8_t depth, TTEntryType type);

        TTWriter(TTEntry* entry, uint8_t gen8) : entry(entry), gen8(gen8) {}
    
    private:
        TTEntry* entry;
        uint8_t gen8;
};


class TTable {
    public:
        TTable();
        ~TTable();

        void resize(size_t size);

        // probe the transposition table for an entry
        // returns a tuple with a boolean indicating if the entry was found
        // and a pointer to the entry.
        // If the entry was not found, the pointer is to the location for the new entry.
        std::tuple<bool, TTData, TTWriter> probe(uint64_t key);

        void new_search();

        void print_stats() const;
        
    private:
        std::unique_ptr<Cluster[]> table;
        size_t size;

        size_t index(uint64_t key) const;
        uint8_t relativeAge(uint8_t generation8) const;

        uint8_t generation8 = 0;

        // stats for the transposition table
        uint hits = 0;
        uint collisions = 0;
};


} // namespace harukashogi

#endif