#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <cstdint>
#include <random>

#include "types.h"

namespace harukashogi {


class OBEntry {
    public:
        OBEntry(uint64_t key, uint64_t data) : key(key), data(data) {}

        // samples a move from the entry based on the counts
        // if the position is not in the entry, returns Move::null()
        uint64_t get_key() const { return key; }
        uint16_t get_move_data(int index) const { return (data >> (index * 16)) & 0xFFFF; }
        uint8_t get_count(int index) const { return (data >> (48 + index * 5)) & 0x1F; }

    private:
        uint64_t key;
        // the 3 most popular moves from the position in the first 48 bits (16 each)
        // then the counts for each move in the next 15 bits (15 each)
        uint64_t data;
};


class OpeningBook {
    public:
        OpeningBook() = default;

        Move sample_move(uint64_t key) const;
    
    private:
        mutable std::mt19937 rng{std::random_device{}()};
};


} // namespace harukashogi

#endif