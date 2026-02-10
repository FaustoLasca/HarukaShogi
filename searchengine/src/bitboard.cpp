#include <bitset>
#include <bit>
#include <immintrin.h>
#include <vector>

#include "bitboard.h"
#include "types.h"
#include "misc.h"

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
Bitboard LineBB[NUM_SQUARES][NUM_SQUARES];


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


std::vector<Bitboard> gen_occupied(Bitboard attacks) {
    std::vector<Bitboard> occupied;
    Bitboard bb = 0;
    Square sq;

    occupied.reserve(1 << popcount(attacks));
    occupied.push_back(bb);

    while (attacks) {
        sq = pop_lsb(attacks);

        size_t current_size = occupied.size();
        for (size_t i = 0; i < current_size; ++i) {
            occupied.push_back(occupied[i] | square_bb(sq));
        }
    }

    return occupied;
}


Bitboard trim_xray(Square sq, Bitboard xray) {
    if (rank_of(sq) != R_9) {
        xray &= ~Rank9BB;
    }
    if (rank_of(sq) != R_1) {
        xray &= ~Rank1BB;
    }
    if (file_of(sq) != F_1) {
        xray &= ~File1BB;
    }
    if (file_of(sq) != F_9) {
        xray &= ~File9BB;
    }

    return xray;
}


// this data structure stores all the necesary information to look up the pext-bitboard in the array
// 1. mask_hi, mask_lo: the attack mask for the given piece and square, split in two 64 bit
//    numbers (pext is only up to 64 bita at a time)
// 2. shift: the shift amount to apply to the result of the hi pext result to concatenate them into
//    a single index of the occupied bitboard
// 3. offset: the offset to apply to the result of the lo pext result to get the final index into
//    data structure
struct PextInfo {
    uint64_t mask_hi, mask_lo;
    uint8_t shift;
    uint32_t offset;
};

PextInfo PextInfos[4][NUM_SQUARES];
Bitboard PextBitboards[520448];


Bitboard sld_attacks_bb(size_t sld_idx, Square from, Bitboard occupied) {
    PextInfo info = PextInfos[sld_idx][from];
    // split the occupied bitboard in lo and hi
    uint64_t occ_lo = static_cast<uint64_t>(occupied);
    uint64_t occ_hi = static_cast<uint64_t>(occupied >> 64);
    
    // compute the pext
    uint64_t lo_pext = _pext_u64(occ_lo, info.mask_lo);
    uint64_t hi_pext = _pext_u64(occ_hi, info.mask_hi);
    uint64_t pext = lo_pext | (hi_pext << info.shift);

    // return the bitboard
    return PextBitboards[info.offset + pext];
}

// helper function used to initialize the pext-bitboards
Bitboard dirty_gen_sld(Piece p, Square sq, Bitboard occupied = 0) {
    switch (p) {
        case B_BISHOP:
            return gen_sld_attacks<BLACK, BISHOP>(sq, occupied);
        case B_ROOK:
            return gen_sld_attacks<BLACK, ROOK>(sq, occupied);
        case B_LANCE:
            return gen_sld_attacks<BLACK, LANCE>(sq, occupied);
        case W_LANCE:
            return gen_sld_attacks<WHITE, LANCE>(sq, occupied);
        default:
            throw std::invalid_argument("Invalid piece");
    }
}


void init_pext_bitboards() {
    size_t sld_idx;
    uint64_t mask_lo, mask_hi;
    uint64_t occ_lo, occ_hi;
    uint64_t pext_lo, pext_hi, pext;
    Bitboard xray;
    Bitboard attacks;

    uint32_t offset = 0;
    for (auto p : {B_BISHOP, B_ROOK, B_LANCE, W_LANCE}) {
        sld_idx = sl_dir_index(p);
        for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {

            // store the offset to the big data structure
            PextInfos[sld_idx][sq].offset = offset;

            // compute the mask Bitboard
            xray = dirty_gen_sld(p, sq, 0);
            xray = trim_xray(sq, xray);

            // split the mask in hi and lo
            mask_lo = static_cast<uint64_t>(xray);
            mask_hi = static_cast<uint64_t>(xray >> 64);
            PextInfos[sld_idx][sq].mask_lo = mask_lo;
            PextInfos[sld_idx][sq].mask_hi = mask_hi;

            // the shift amount is equal to the number of bits in the lo mask
            PextInfos[sld_idx][sq].shift = popcount(mask_lo);

            // generate all the relevant occupied bitboards
            auto occupied = gen_occupied(xray);

            // fill the data structure with the pext-bitboards
            for (auto occ : occupied) {
                // compute the attacks
                attacks = dirty_gen_sld(p, sq, occ);

                // split the occupied bitboard in lo and hi
                occ_lo = static_cast<uint64_t>(occ);
                occ_hi = static_cast<uint64_t>(occ >> 64);

                // compute the pext
                pext_lo = _pext_u64(occ_lo, mask_lo);
                pext_hi = _pext_u64(occ_hi, mask_hi);
                pext = pext_lo | (pext_hi << PextInfos[sld_idx][sq].shift);

                // store the attacks in the data structure
                PextBitboards[PextInfos[sld_idx][sq].offset + pext] = attacks;

                // increment the offset
                offset++;
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
            LineBB[from][to] = 0;

            for (int i = 0; i < 8; ++i) {
                Direction d = dirs[i];
                Bitboard from_bb = square_bb(from);
                Bitboard to_bb = square_bb(to);

                Bitboard between = 0;

                while (from_bb && !(between & to_bb)) {
                    from_bb = dir_attacks_bb(from_bb, d);
                    between |= from_bb;
                }

                if (between & to_bb) {
                    BetweenBB[from][to] = between ^ to_bb;

                    // LineBB: the full line through both squares
                    Direction opp = dirs[(i + 4) % 8];
                    Bitboard line = square_bb(from) | square_bb(to) | BetweenBB[from][to];

                    // extend from 'from' in the opposite direction
                    Bitboard walker = square_bb(from);
                    while (walker) {
                        walker = dir_attacks_bb(walker, opp);
                        line |= walker;
                    }

                    // extend from 'to' in the original direction
                    walker = square_bb(to);
                    while (walker) {
                        walker = dir_attacks_bb(walker, d);
                        line |= walker;
                    }

                    LineBB[from][to] = line;
                    break;
                }
            }
        }
    }
}


Bitboard dir_attacks_bb(Square from, Color c, PieceType pt) {
    return PieceDirAttacksBB[c][pt][from];
}


// Bitboard sld_attacks_bb(Square from, Color c, PieceType pt, Bitboard occupied) {
//     size_t index = sl_dir_index(make_piece(c, pt));
//     Bitboard xray = PieceSldAttacksBB[index][from][0];
//     xray = trim_xray(from, xray);
//     return PieceSldAttacksBB[index][from][occupied & xray];
// }


Bitboard between_bb(Square from, Square to) {
    return BetweenBB[from][to];
}


Bitboard line_bb(Square from, Square to) {
    return LineBB[from][to];
}


// initializes the precomputed data structures for bitboards
void Bitboards::init() {
    init_piece_dir_attacks();
    init_pext_bitboards();
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