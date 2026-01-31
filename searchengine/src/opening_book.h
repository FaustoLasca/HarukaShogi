#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <cstdint>

#include "types.h"

namespace harukashogi {


class OBEntry {
    public:
        OBEntry(uint64_t key, uint64_t data) : key(key), data(data) {}

        // adds a move to the entry
        // returns true if the move was added, false if the entry is full
        bool add_move(Move move);

    private:
        uint64_t key;
        // the 3 most popular moves from the position in the first 48 bits (16 each)
        // then the counts for each move in the next 15 bits (15 each)
        uint64_t data;
};


} // namespace harukashogi

#endif