#include <iostream>

#include "position.h"
#include "engine.h"
#include "misc.h"

using namespace harukashogi;

int main() {
    init();
    Position pos;
    pos.set("k8/9/6b2/4gs3/4p4/3GG4/2B3B2/9/8K b - 1");
    std::cout << pos.sfen() << std::endl;
    Move m = move_from_string("5f5e");
    std::cout << pos.see_ge(m, 0) << std::endl;
    pos.make_move(m);
    std::cout << pos.sfen() << std::endl;
}