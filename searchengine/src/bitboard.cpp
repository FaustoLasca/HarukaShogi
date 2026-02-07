#include <bitset>
#include <bit>

#include "bitboard.h"
#include "types.h"

namespace harukashogi {


int popcount(Bitboard bb) {
    return std::popcount(bb);
}

bool one_bit(Bitboard bb) {
    return std::has_single_bit(bb);
}

Square lsb(Bitboard bb) {
    return Square(std::countr_zero(bb));
}

Square pop_lsb(Bitboard& bb) {
    Square sq = Square(std::countr_zero(bb));
    bb &= bb - 1;
    return sq;
}


// data structures for precomputed bitboards
Bitboard PieceDirAttacksBB[NUM_COLORS][NUM_PIECE_TYPES][NUM_SQUARES];
Bitboard BetweenBB[NUM_SQUARES][NUM_SQUARES];


void init_piece_dir_attacks() {
    Bitboard bb, attacks;
    Piece p;
    Direction d;

    // loop through all the elements of the data structure
    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (PieceType pt = KING; pt < NUM_PIECE_TYPES; ++pt) {
            p = make_piece(c, pt);

            for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {

                bb = square_bb(sq);
                attacks = 0;

                for (int i = 0; i < 8 && PTDirections[p-1][i] != NULL_DIR; ++i) {
                    d = PTDirections[p-1][i];
                    switch (d) {
                        case N_DIR:
                            attacks |= dir_attacks_bb<N_DIR>(bb);
                            break;
                        case NE_DIR:
                            attacks |= dir_attacks_bb<NE_DIR>(bb);
                            break;
                        case E_DIR:
                            attacks |= dir_attacks_bb<E_DIR>(bb);
                            break;
                        case SE_DIR:
                            attacks |= dir_attacks_bb<SE_DIR>(bb);
                            break;
                        case S_DIR:
                            attacks |= dir_attacks_bb<S_DIR>(bb);
                            break;
                        case SW_DIR:
                            attacks |= dir_attacks_bb<SW_DIR>(bb);
                            break;
                        case W_DIR:
                            attacks |= dir_attacks_bb<W_DIR>(bb);
                            break;
                        case NW_DIR:
                            attacks |= dir_attacks_bb<NW_DIR>(bb);
                            break;
                        case NNE_DIR:
                            attacks |= dir_attacks_bb<NNE_DIR>(bb);
                            break;
                        case NNW_DIR:
                            attacks |= dir_attacks_bb<NNW_DIR>(bb);
                            break;
                        case SSE_DIR:
                            attacks |= dir_attacks_bb<SSE_DIR>(bb);
                            break;
                        case SSW_DIR:
                            attacks |= dir_attacks_bb<SSW_DIR>(bb);
                        default:
                            break;
                    }

                    PieceDirAttacksBB[c][pt][sq] = attacks;
                }
            }
        }
    }
}


void init_between_bb() {
    Direction dirs[] = {N_DIR, NE_DIR, E_DIR, SE_DIR, S_DIR, SW_DIR, W_DIR, NW_DIR};
    // loop through all combinations of squares and store the associated bitboard
    for (Square from = SQ_11; from < NUM_SQUARES; ++from) {
        for (Square to = SQ_11; to < NUM_SQUARES; ++to) {

            BetweenBB[from][to] = 0;

            for (Direction d : dirs) {
                Bitboard from_bb = square_bb(from);
                Bitboard to_bb = square_bb(to);

                Bitboard between = 0;

                while (from_bb && !(between & to_bb)) {
                    from_bb = dir_attacks_bb(from_bb, d);
                    between |= from_bb;
                }

                if (between & to_bb) {
                    BetweenBB[from][to] = between ^ to_bb;
                    break;
                }
            }
        }
    }
}


Bitboard dir_attacks_bb(Bitboard from, Direction d) {
    switch (d) {
        case N_DIR:
            return dir_attacks_bb<N_DIR>(from);
        case NE_DIR:
            return dir_attacks_bb<NE_DIR>(from);
        case E_DIR:
            return dir_attacks_bb<E_DIR>(from);
        case SE_DIR:
            return dir_attacks_bb<SE_DIR>(from);
        case S_DIR:
            return dir_attacks_bb<S_DIR>(from);
        case SW_DIR:
            return dir_attacks_bb<SW_DIR>(from);
        case W_DIR:
            return dir_attacks_bb<W_DIR>(from);
        case NW_DIR:
            return dir_attacks_bb<NW_DIR>(from);
        case NNE_DIR:
            return dir_attacks_bb<NNE_DIR>(from);
        case NNW_DIR:
            return dir_attacks_bb<NNW_DIR>(from);
        case SSE_DIR:
            return dir_attacks_bb<SSE_DIR>(from);
        case SSW_DIR:
            return dir_attacks_bb<SSW_DIR>(from);
        default:
            return 0;
    }
}


Bitboard dir_attacks_bb(Square from, Color c, PieceType pt) {
    return PieceDirAttacksBB[c][pt][from];
}


Bitboard between_bb(Square from, Square to) {
    return BetweenBB[from][to];
}


// initializes the precomputed data structures for bitboards
void Bitboards::init() {
    init_piece_dir_attacks();
    init_between_bb();
}



std::ostream& operator<<(std::ostream& os, const Bitboard& bb) {
    std::bitset<9> row;
    for (int rank = R_1; rank < NUM_RANKS; ++rank) {
        row = bb >> (rank * 9) & 0x1FFull;
        for (int file = F_9; file >= F_1; --file) {
            os << row[file] << " ";
        }
        os << std::endl;
    }
    return os;
};


} // namespace harukashogi