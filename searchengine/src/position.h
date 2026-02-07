#ifndef POSITION_H
#define POSITION_H

#include <string>
#include <array>
#include <forward_list>
#include <iostream>

#include "types.h"
#include "bitboard.h"

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


struct StateInfo {
	StateInfo() : capturedPT(NO_PIECE_TYPE),
				  checkersBB(0),
				  key(0) {}

	Bitboard checkersBB;
	Bitboard blockers[NUM_COLORS];

	PieceType capturedPT;
	uint64_t key;
};


constexpr uint8_t MAX_REPETITIONS = 4;

class RepetitionTable {
	public:
		RepetitionTable() = default;

		void add(uint64_t key) { table[index(key)]++;}
		void remove(uint64_t key) { table[index(key)]--;}

		bool reached_repetitions(
			uint64_t key,
			std::forward_list<StateInfo>& si,
			uint8_t nRepetitions = MAX_REPETITIONS);

		int get_counts_needed() const { return countsNeeded; }
		int get_repetitions() const { return repetitions; }
		
		private:
		// table size is 2^14
		// the first 14 bits of the key are used to index
		uint8_t table[16384] = {};
		int countsNeeded = 0;
		int repetitions = 0;
		
		size_t index(uint64_t key) const { return key & 0x3FFFull; }
};


constexpr uint8_t MAX_HAND_COUNT = 18;


class Position {
    public:
		// constructor
		Position() = default;

		//initialization
		static void init();

		// SFEN string methods
		void set(const std::string& 
			sfenStr = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
		);
		std::string sfen() const;

		// move methods
		void make_move(Move m);
		void unmake_move(Move m);

		Bitboard attackers_to(Square sq, Bitboard occupied) const;
		Bitboard checkers() const { return si.front().checkersBB; }
		
		bool is_checkmate();
		bool is_game_over();
		
		bool is_legal(Move m);
		bool is_capture(Move m) const;

		// getters
		Piece piece(Square sq) const { return board[sq]; }
		Square king_square(Color color) const { return kingSq[color]; }
		int hand_count(Color color, PieceType pt) const {
			return hands[color][pt];
		}
		Color side_to_move() const { return sideToMove; }
		bool pawn_on_file(Color color, File file) const { 
			return pawnFiles[color][file];
		}
		Color get_winner() const;
		int get_move_count() const { return gamePly; }
		uint64_t get_key() const { return si.front().key; }

		Bitboard all_pieces(Color color) const { return allPiecesBB[color]; }
		Bitboard all_pieces() const { return allPiecesBB[BLACK] | allPiecesBB[WHITE]; }
		Bitboard dir_pieces(Color color, Direction dir) const { return dirPieces[color][dir]; }
		Bitboard sld_pieces(Color color, PieceType pt) const {
			return slPieces[color][sliding_type_index(pt)];
		}

		// temporary method to debug the repetition table
		void print_repetition_values() const {
			std::cout << "counts needed: " << repetitionTable.get_counts_needed() << std::endl;
			std::cout << "repetitions: " << repetitionTable.get_repetitions() << std::endl;
		}

		
		private:

		// methods to update all the affected information when a piece moves
		void add_piece(Piece p, Square sq);
		void remove_piece(Square sq);
		void move_piece(Square from, Square to);
		void add_hand_piece(Color color, PieceType pt);
		void remove_hand_piece(Color color, PieceType pt);

		void update_blocker_info(Color c);

		// compute the zobrist hash code
		void compute_key();
		
		// repetition table
		RepetitionTable repetitionTable;
		
		// data members
		std::array<Piece, NUM_SQUARES> board;
		uint8_t hands[NUM_COLORS][NUM_UNPROMOTED_PIECE_TYPES] = {};

		// bitboards
		Bitboard allPiecesBB[NUM_COLORS] = {};
		Bitboard dirPieces[NUM_COLORS][NUM_DIRECTIONS] = {};
		Bitboard slPieces[NUM_COLORS][NUM_SLIDING_TYPES] = {};
		std::array<Square, NUM_COLORS> kingSq;

		bool pawnFiles[NUM_COLORS][NUM_FILES] = {};
		Color sideToMove;
		int gamePly;
		
		GameStatus gameStatus;
		Color winner;
		
		std::forward_list<StateInfo> si;
};


} // namespace harukashogi

#endif // POSITION_H