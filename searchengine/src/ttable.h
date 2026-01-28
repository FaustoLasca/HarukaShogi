#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include <array>

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
// 128 bits, 16 bytes in total
struct TTEntry {
    uint64_t key;
    int score;
    Move bestMove;
    uint8_t depth;
    NodeType nodeType;
};


class TranspositionTable {
    public:
        TranspositionTable() = default;

        // probe the transposition table for an entry
        // returns a tuple with a boolean indicating if the entry was found
        // and a pointer to the entry.
        // If the entry was not found, the pointer is to the location for the new entry.
        std::tuple<bool, TTEntry*> probe(uint64_t key);
    private:
        // uses the bottom 20 bits of the key to index the table
        // 2^22 = 4 194 304 entries, 16 bytes each = 64 MB
        std::array<TTEntry, 4194304> table;
        size_t index(uint64_t key) const { return key & 0x3FFFFFull; }

        // stats for the transposition table
        uint count = 0;
        uint hits = 0;
        uint collisions = 0;
};


} // namespace harukashogi

#endif