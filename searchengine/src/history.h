#ifndef HISTORY_H
#define HISTORY_H

#include <algorithm>

namespace harukashogi {


constexpr int HISTORY_SIZE = 1ull << 16;

constexpr int HISTORY_MAX_VALUE = 30000;

struct HistoryEntry {
    private:
        int16_t value = 0;

    public:
        void operator= (const int value) {
            this->value = value;
        }

        operator int() const { return value; }

        // this keeps the history value in the range of -HISTORY_MAX_VALUE to HISTORY_MAX_VALUE
        void operator << (int bonus) {
            int clamped = std::clamp(bonus, -HISTORY_MAX_VALUE, HISTORY_MAX_VALUE);
            value = value + clamped - value * std::abs(clamped) / HISTORY_MAX_VALUE;
        };
};


} // namespace harukashogi

#endif // HISTORY_H