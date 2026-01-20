# include "test.h"
# include <iostream>

Test::Test(int value) : value(value) {}

int Test::search(int depth) {
    std::cout << "Searching at depth: " << depth << std::endl;
    return value + depth;
}