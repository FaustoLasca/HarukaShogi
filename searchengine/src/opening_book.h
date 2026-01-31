#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <cstdint>

#include "types.h"

namespace harukashogi {


class OBEntry {
    public:
        OBEntry(uint64_t key, uint64_t data) : key(key), data(data) {}

        // samples a move from the entry based on the counts
        // if the position is not in the entry, returns Move::null()
        Move sample_move() const;

        uint64_t get_key() const { return key; }

    private:
        uint64_t key;
        // the 3 most popular moves from the position in the first 48 bits (16 each)
        // then the counts for each move in the next 15 bits (15 each)
        uint64_t data;

        uint16_t get_move_data(int index) const { return (data >> (index * 16)) & 0xFFFF; }
        uint8_t get_count(int index) const { return (data >> (48 + index * 5)) & 0x1F; }
};


class OpeningBook {
    public:
        OpeningBook() {};

        Move sample_move(uint64_t key) const;
};


} // namespace harukashogi

#endif