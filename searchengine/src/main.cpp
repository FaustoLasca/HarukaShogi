#include <iostream>
#include <bitset>

#include "bitboard.h"
#include "misc.h"
#include "position.h"
#include "perft.h"
#include "types.h"

using namespace harukashogi;


int main() {
    Position::init();

    Position pos;
    pos.set();

    perft(pos, 3);
    
    std::cout << "BLACK N:\n" << pos.dir_pieces(BLACK, N_DIR) << std::endl;
    std::cout << "BLACK NE_DIR:\n" << pos.dir_pieces(BLACK, NE_DIR) << std::endl;
    std::cout << "BLACK E_DIR:\n" << pos.dir_pieces(BLACK, E_DIR) << std::endl;
    std::cout << "BLACK SE_DIR:\n" << pos.dir_pieces(BLACK, SE_DIR) << std::endl;
    std::cout << "BLACK S_DIR:\n" << pos.dir_pieces(BLACK, S_DIR) << std::endl;
    std::cout << "BLACK SW_DIR:\n" << pos.dir_pieces(BLACK, SW_DIR) << std::endl;
    std::cout << "BLACK W_DIR:\n" << pos.dir_pieces(BLACK, W_DIR) << std::endl;
    std::cout << "BLACK NW_DIR:\n" << pos.dir_pieces(BLACK, NW_DIR) << std::endl;
    std::cout << "BLACK NNE_DIR:\n" << pos.dir_pieces(BLACK, NNE_DIR) << std::endl;
    std::cout << "BLACK NNW_DIR:\n" << pos.dir_pieces(BLACK, NNW_DIR) << std::endl;
    std::cout << "BLACK SSE_DIR:\n" << pos.dir_pieces(BLACK, SSE_DIR) << std::endl;
    std::cout << "BLACK SSW_DIR:\n" << pos.dir_pieces(BLACK, SSW_DIR) << std::endl;

    std::cout << "WHITE N:\n" << pos.dir_pieces(WHITE, N_DIR) << std::endl;
    std::cout << "WHITE NE_DIR:\n" << pos.dir_pieces(WHITE, NE_DIR) << std::endl;
    std::cout << "WHITE E_DIR:\n" << pos.dir_pieces(WHITE, E_DIR) << std::endl;
    std::cout << "WHITE SE_DIR:\n" << pos.dir_pieces(WHITE, SE_DIR) << std::endl;
    std::cout << "WHITE S_DIR:\n" << pos.dir_pieces(WHITE, S_DIR) << std::endl;
    std::cout << "WHITE SW_DIR:\n" << pos.dir_pieces(WHITE, SW_DIR) << std::endl;
    std::cout << "WHITE W_DIR:\n" << pos.dir_pieces(WHITE, W_DIR) << std::endl;
    std::cout << "WHITE NW_DIR:\n" << pos.dir_pieces(WHITE, NW_DIR) << std::endl;
    std::cout << "WHITE NNE_DIR:\n" << pos.dir_pieces(WHITE, NNE_DIR) << std::endl;
    std::cout << "WHITE NNW_DIR:\n" << pos.dir_pieces(WHITE, NNW_DIR) << std::endl;
    std::cout << "WHITE SSE_DIR:\n" << pos.dir_pieces(WHITE, SSE_DIR) << std::endl;
    std::cout << "WHITE SSW_DIR:\n" << pos.dir_pieces(WHITE, SSW_DIR) << std::endl;

    return 0;
}