#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <string>

#include "types.h"

namespace harukashogi {


enum GameStatus {
    NO_STATUS,
    IN_PROGRESS,
    GAME_OVER
};


enum CheckStatus {
    NOT_CHECK,
    CHECK,
    CHECK_UNRESOLVED
};


class Position {
    public:
      // constructor
      Position() = default;

      // SFEN string methods
      void set(const std::string& sfenStr);
      std::string sfen() const;

      // move methods
      void make_move(Move m);
      void unmake_move(Move m);

      
      bool is_in_check(Color color);
      bool is_legal(Move m);
      bool is_checkmate();
      bool is_game_over();
      

      // getters
      Piece piece(Square sq) const { return board[sq]; }
      int hand_count(Color color, PieceType pt) const { return hands[color * NUM_UNPROMOTED_PIECE_TYPES + pt]; }
      Color side_to_move() const { return sideToMove; }
      bool pawn_on_file(Color color, File file) const { return pawnFiles[color * NUM_FILES + file]; }
      Color get_winner() const;
      int get_move_count() const { return gamePly; }

    private:
      // data members
      std::array<Piece, NUM_SQUARES> board;
      std::array<uint8_t, NUM_COLORS * NUM_UNPROMOTED_PIECE_TYPES> hands;
      std::array<Square, NUM_COLORS> kingSq;
      std::array<bool, NUM_COLORS * NUM_FILES> pawnFiles;
      Color sideToMove;
      int gamePly;
      GameStatus gameStatus;
      std::array<CheckStatus, NUM_COLORS> checkStatus;
      Color winner;
};

} // namespace harukashogi

#endif // POSITION_H