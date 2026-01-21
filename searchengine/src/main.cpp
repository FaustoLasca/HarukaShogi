#include <iostream>
#include "types.h"


int main() {
    Color color = Color::BLACK;
    std::cout << "Color: " << (color == Color::BLACK ? "BLACK" : "WHITE") << std::endl;
    return 0;
}