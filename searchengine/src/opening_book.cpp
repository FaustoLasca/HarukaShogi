#include <algorithm>
#include <random>

#include "opening_book.h"
#include "incbin.h"

namespace harukashogi {


// include the opening book data from the binary file
INCBIN(OBEntry, Book, "../bin/book_data.bin");


OpeningBook::OpeningBook() : size(gBookSize / sizeof(OBEntry)) {}


Move OpeningBook::sample_move(uint64_t key) const {
    const OBEntry* it = std::lower_bound(
        gBookData,
        gBookData + this->size,
        key,
        [](const OBEntry& entry, uint64_t key) {
            return entry.get_key() < key;
        }
    );

    if (it == gBookData + this->size || it->get_key() != key) {
        return Move::null();
    }

    // sample a move from the entry
    uint8_t counts[3];
    uint8_t max_count = it->get_count(0);
    int total_count = 0;

    for (int i = 0; i < 3; i++) {
        counts[i] = it->get_count(i);
        total_count += counts[i];
    }

    int rn = rng() % total_count;
    for (int i = 0; i < 3; i++) {
        if (rn < counts[i]) {
            return Move(it->get_move_data(i));
        }
        rn -= counts[i];
    }
    return Move::null();
}


} // namespace harukashogi