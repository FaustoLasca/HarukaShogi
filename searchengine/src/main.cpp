#include <iostream>
#include <string>

#include "types.h"
#include "position.h"


using namespace harukashogi;

int main() {
    Position pos;
    pos.set("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
    std::cout << "It lives!" << std::endl;
    return 0;
}