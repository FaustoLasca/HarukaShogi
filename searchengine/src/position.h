#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <string>
#include <vector>
#include <iostream>

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


constexpr uint8_t MAX_REPETITIONS = 4;

class RepetitionTable {
	public:
		RepetitionTable() {
			keyHistory.reserve(512);
		};

		void add(uint64_t key) { table[index(key)]++; keyHistory.push_back(key); }
		void remove(uint64_t key) { table[index(key)]--; keyHistory.pop_back(); }

		bool reached_repetitions(uint64_t key, uint8_t nRepetitions = MAX_REPETITIONS);

		int get_counts_needed() const { return countsNeeded; }
		int get_repetitions() const { return repetitions; }
		
		private:
		// table size is 2^14
		// the first 14 bits of the key are used to index
		uint8_t table[16384] = {};
		std::vector<uint64_t> keyHistory;
		int countsNeeded = 0;
		int repetitions = 0;
		
		size_t index(uint64_t key) const { return key >> 50; }
};


constexpr uint8_t MAX_HAND_COUNT = 18;

class Position {
    public:
		// constructor
		Position() = default;

		//initialization
		static void init();

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
		uint64_t get_key() const { return key; }

		// temporary method to debug the repetition table
		void print_repetition_values() const {
			std::cout << "counts needed: " << repetitionTable.get_counts_needed() << std::endl;
			std::cout << "repetitions: " << repetitionTable.get_repetitions() << std::endl;
		}

		
		private:
		// zobrist hash code
		void compute_key();
		uint64_t key;

		// repetition table
		RepetitionTable repetitionTable;

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