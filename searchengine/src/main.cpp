#include <iostream>

#include "engine.h"
#include "misc.h"

using namespace harukashogi;


int main() {
    init();

    // std::string moveStr = "7g7f";
    // Move move = move_from_string(moveStr);
    // std::cout << move << std::endl;

    USIEngine engine(8);
    engine.run();

    return 0;
}