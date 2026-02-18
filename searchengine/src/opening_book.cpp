#include <algorithm>
#include <iostream>
#include <random>

#include "opening_book.h"
#include "book_data.h"
#include "misc.h"

namespace harukashogi {


Move OpeningBook::sample_move(uint64_t key) const {
    const OBEntry* it = std::lower_bound(
        BookData,
        BookData + OPENING_BOOK_SIZE,
        key,
        [](const OBEntry& entry, uint64_t key) {
            return entry.get_key() < key;
        }
    );

    if (it == BookData + OPENING_BOOK_SIZE || it->get_key() != key) {
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