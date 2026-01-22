#include <sstream>
#include <iostream>

#include "position.h"
#include "types.h"

namespace harukashogi {

constexpr std::string_view PieceToChar(" kgslnbrp      KGSLNBRP");

// initializes the position from a SFEN string
void Position::set(const std::string& sfenStr) {
    char token;
    size_t idx;
    bool promote = false;
    Color color;
    Square sq = SQ_91;
    std::istringstream ss(sfenStr);

    ss >> std::noskipws;

    // 1. board pieces
    while ((ss >> token) && token != ' ') {
        if (isdigit(token))
            sq += (token - '0') * EAST;

        else if (token == '/')
            sq += 2 * SOUTH;

        else if (token == '+')
            promote = true;

        else if ((idx = PieceToChar.find(token)) != std::string::npos) {
            
            board[sq] = promote ? promote_piece(Piece(idx)) : Piece(idx);
            promote = false;
            sq += EAST;
        }
    }

    // 2. side to move
    ss >> token;
    sideToMove = (token == 'b') ? BLACK : WHITE;
    ss >> token;

    // 3. hand pieces
    while ((ss >> token) && token != ' ') {
        color = isupper(token) ? WHITE : BLACK;
        if ((idx = PieceToChar.find(tolower(token))) != std::string::npos)
            hands[color * NUM_UNPROMOTED_PIECE_TYPES + type_of(Piece(idx))]++;
    }

    // 4. full move count
    ss >> std::skipws >> gamePly;
}

} // namespace harukashogi