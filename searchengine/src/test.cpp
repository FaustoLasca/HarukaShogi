#include <iostream>

#include "movepicker.h"
#include "position.h"
#include "engine.h"
#include "misc.h"

using namespace harukashogi;

int main() {
    init();

    Position pos;
    pos.set("l7l/1r2gkg2/2n1p1np1/p1ppssp1p/1p3p3/P1P1S1P1P/1PSPP1N2/2G2G3/LNK4RL b BPbp 1");
    HistoryEntry moveHistory[MAX_MOVES];
    MovePicker mp(pos, 1, moveHistory, move_from_string("B*2e"));

    Move m;
    while (!(m = mp.next_move()).is_null()) {
        std::cout << m << (pos.is_capture(m) ? " capture" : " quiet") << std::endl;
    }
}