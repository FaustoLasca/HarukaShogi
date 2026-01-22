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

    private:
      // data members
      std::array<Piece, NUM_SQUARES> board;
      std::array<uint8_t, NUM_COLORS * NUM_UNPROMOTED_PIECE_TYPES> hands;
      Color sideToMove;
      int gamePly;
};

} // namespace harukashogi

#endif // POSITION_H