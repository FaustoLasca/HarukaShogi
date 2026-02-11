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
enum NodeType : uint8_t {
    PV_NODE,
    CUT_NODE,
    ALL_NODE
};


// an entry in the transposition table
// 64 bits, 8 bytes in total
struct TTEntry {
    uint16_t key = 0;
    int16_t score = 0;
    Move bestMove = Move::null();
    uint8_t depth = 0;
    NodeType nodeType = ALL_NODE;

    TTEntry() = default;
};


struct Cluster;


class TTable {
    public:
        TTable();
        ~TTable();

        // probe the transposition table for an entry
        // returns a tuple with a boolean indicating if the entry was found
        // and a pointer to the entry.
        // If the entry was not found, the pointer is to the location for the new entry.
        std::tuple<bool, TTEntry*> probe(uint64_t key);

        void print_stats() const;
        
        private:
        std::unique_ptr<Cluster[]> table;
        size_t index(uint64_t key, uint64_t ttSize) const;

        // stats for the transposition table
        uint count = 0;
        uint hits = 0;
        uint collisions = 0;
};


} // namespace harukashogi

#endif