#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <string>

#include "types.h"

namespace harukashogi {

class Position {
    public:
      // constructor
      Position() = default;

      // SFEN string methods
      void set(const std::string& sfenStr);
      std::string sfen() const;

      // move methods
      void make_move(Move m);
      void undo_move(Move m);

      // checkers
      bool is_in_check(Color color) const;
      bool is_legal(Move m);

      // getters
      Piece piece(Square sq) const { return board[sq]; }

    private:
      // data members
      std::array<Piece, NUM_SQUARES> board;
      std::array<uint8_t, NUM_COLORS * NUM_UNPROMOTED_PIECE_TYPES> hands;
      std::array<Square, NUM_COLORS> kingSq;
      Color sideToMove;
      int gamePly;
};

} // namespace harukashogi

#endif // POSITION_H