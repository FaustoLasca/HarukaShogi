#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>
#include <cassert>
#include <stdexcept>

namespace harukashogi {

enum Color : uint8_t {
    BLACK,
    WHITE,

    NO_COLOR,

    NUM_COLORS = 2,
};

constexpr Color operator~(Color c) { return Color(1 - int(c)); };

enum PieceType : uint8_t {
    KING,
    GOLD,
    SILVER,
    LANCE,
    KNIGHT,
    BISHOP,
    ROOK,
    PAWN,

    P_SILVER,
    P_LANCE,
    P_KNIGHT,
    P_BISHOP,
    P_ROOK,
    P_PAWN,

    NUM_PIECE_TYPES = 14,

    NO_PIECE_TYPE,
};
constexpr uint8_t NUM_SLIDING_TYPES = 5;
constexpr uint8_t NUM_UNPROMOTED_PIECE_TYPES = 8;

enum Piece : uint8_t {
    NO_PIECE,

    B_KING, B_GOLD, B_SILVER, B_LANCE, B_KNIGHT, B_BISHOP, B_ROOK, B_PAWN,
    P_B_SILVER, P_B_LANCE, P_B_KNIGHT, P_B_BISHOP, P_B_ROOK, P_B_PAWN,

    W_KING, W_GOLD, W_SILVER, W_LANCE, W_KNIGHT, W_BISHOP, W_ROOK, W_PAWN,
    P_W_SILVER, P_W_LANCE, P_W_KNIGHT, P_W_BISHOP, P_W_ROOK, P_W_PAWN,

    NUM_PIECES = 28,
    NUM_TOT_PIECES = 40,
};

// functions on pieces and piece types
constexpr PieceType type_of(Piece p) { return PieceType( (p-1) % NUM_PIECE_TYPES ); };
constexpr Color color_of(Piece p) { return Color( (p-1) / NUM_PIECE_TYPES ); };
constexpr Piece make_piece(Color c, PieceType pt) { return Piece(int(c) * int(NUM_PIECE_TYPES) + int(pt) + 1); };
constexpr bool is_promoted( PieceType pt ) { return pt >= P_SILVER; };
constexpr bool is_promoted( Piece p ) { return is_promoted(type_of(p)); };
constexpr bool can_promote( PieceType pt ) { return pt>=SILVER && pt<=PAWN; };
constexpr Piece promote_piece( Piece p ) { return is_promoted(p) ? p : Piece(p + 6); };
constexpr Piece unpromote_piece( Piece p ) { return is_promoted(p) ? Piece(p - 6) : p; };
constexpr PieceType unpromoted_type( PieceType pt ) {
    return is_promoted(pt) ? PieceType(pt - 6) : pt;
}
// used to index the sliding pieces bitboard array
constexpr std::size_t sliding_type_index( PieceType pt ) { 
    switch (pt) {
        case BISHOP:
            return 0;
        case P_BISHOP:
            return 1;
        case ROOK:
            return 2;
        case P_ROOK:
            return 3;
        case LANCE:
            return 4;
        default:
            return -1;
    }
 };

enum Square : uint8_t {
    SQ_11, SQ_21, SQ_31, SQ_41, SQ_51, SQ_61, SQ_71, SQ_81, SQ_91,
    SQ_12, SQ_22, SQ_32, SQ_42, SQ_52, SQ_62, SQ_72, SQ_82, SQ_92,
    SQ_13, SQ_23, SQ_33, SQ_43, SQ_53, SQ_63, SQ_73, SQ_83, SQ_93,
    SQ_14, SQ_24, SQ_34, SQ_44, SQ_54, SQ_64, SQ_74, SQ_84, SQ_94,
    SQ_15, SQ_25, SQ_35, SQ_45, SQ_55, SQ_65, SQ_75, SQ_85, SQ_95,
    SQ_16, SQ_26, SQ_36, SQ_46, SQ_56, SQ_66, SQ_76, SQ_86, SQ_96,
    SQ_17, SQ_27, SQ_37, SQ_47, SQ_57, SQ_67, SQ_77, SQ_87, SQ_97,
    SQ_18, SQ_28, SQ_38, SQ_48, SQ_58, SQ_68, SQ_78, SQ_88, SQ_98,
    SQ_19, SQ_29, SQ_39, SQ_49, SQ_59, SQ_69, SQ_79, SQ_89, SQ_99,
    NO_SQUARE,

    NUM_SQUARES = 81,
};

enum File : uint8_t {
    F_1, F_2, F_3, F_4, F_5, F_6, F_7, F_8, F_9,
    NUM_FILES = 9,
};

enum Rank : uint8_t {
    R_1, R_2, R_3, R_4, R_5, R_6, R_7, R_8, R_9,
    NUM_RANKS = 9,
};

enum Direction : int8_t {
    N_DIR,
    NE_DIR,
    E_DIR,
    SE_DIR,
    S_DIR,
    SW_DIR,
    W_DIR,
    NW_DIR,
    // knight directions
    NNE_DIR,
    NNW_DIR,
    SSE_DIR,
    SSW_DIR,

    NULL_DIR,
    NUM_DIRECTIONS = 12,
};

constexpr int dir_delta(Direction dir) {
    switch (dir) {
        case N_DIR:
            return -9;
        case NE_DIR:
            return -10;
        case E_DIR:
            return -1;
        case SE_DIR:
            return 8;
        case S_DIR:
            return 9;
        case SW_DIR:
            return 10;
        case W_DIR:
            return 1;
        case NW_DIR:
            return -8;
        case NNE_DIR:
            return -19;
        case NNW_DIR:
            return -17;
        case SSE_DIR:
            return 17;
        case SSW_DIR:
            return 19;
        default:
            throw std::invalid_argument("Invalid direction");
    }
}

constexpr Square operator+(Square sq, int delta) { return Square(int(sq) + delta); }
constexpr Square operator-(Square sq, int delta) { return Square(int(sq) - delta); }
constexpr Square& operator+=(Square& sq, int delta) { return sq = sq + delta; }
constexpr Square& operator-=(Square& sq, int delta) { return sq = sq - delta; }


// array containing the directions each piece type can move to
constexpr Direction PTDirections[NUM_PIECES][8] = {
    // black pieces
    {N_DIR,    NE_DIR,   E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NW_DIR  }, // KING
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // GOLD
    {N_DIR,    NE_DIR,   SE_DIR,   SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR}, // SILVER
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // LANCE
    {NNE_DIR,  NNW_DIR,  NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // KNIGHT
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // BISHOP
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // ROOK
    {N_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // PAWN
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_SILVER
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_LANCE
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_KNIGHT
    {N_DIR,    E_DIR,    S_DIR,    W_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_BISHOP
    {NE_DIR,   SE_DIR,   SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_ROOK
    {N_DIR,    NE_DIR,   E_DIR,    S_DIR,    W_DIR,    NW_DIR,   NULL_DIR, NULL_DIR}, // P_PAWN

    // white pieces
    {N_DIR,    NE_DIR,   E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NW_DIR  }, // KING
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // GOLD
    {NE_DIR,   SE_DIR,   S_DIR,    SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR}, // SILVER
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // LANCE
    {SSE_DIR,  SSW_DIR,  NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // KNIGHT
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // BISHOP
    {NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // ROOK
    {S_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // PAWN
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_SILVER
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_LANCE
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_KNIGHT
    {N_DIR,    E_DIR,    S_DIR,    W_DIR,    NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_BISHOP
    {NE_DIR,   SE_DIR,   SW_DIR,   NW_DIR,   NULL_DIR, NULL_DIR, NULL_DIR, NULL_DIR}, // P_ROOK
    {N_DIR,    E_DIR,    SE_DIR,   S_DIR,    SW_DIR,   W_DIR,    NULL_DIR, NULL_DIR}, // P_PAWN
};

// array containing the sliding directions each piece type can move to
constexpr Direction PSlidingDirections[NUM_PIECES][4] = {
    {NE_DIR,   SE_DIR,   SW_DIR,   NW_DIR  }, // BISHOP
    {N_DIR,    E_DIR,    S_DIR,    W_DIR   }, // ROOK
    {N_DIR,    NULL_DIR, NULL_DIR, NULL_DIR}, // B_LANCE
    {S_DIR,    NULL_DIR, NULL_DIR, NULL_DIR}  // W_LANCE
};

// returns the index to the PSlidingDirections structure
constexpr size_t sl_dir_index(Piece p) {
    switch (p) {
        case B_BISHOP:
        case P_B_BISHOP:
        case W_BISHOP:
        case P_W_BISHOP:
            return 0;

        case B_ROOK:
        case P_B_ROOK:
        case W_ROOK:
        case P_W_ROOK:
            return 1;

        case B_LANCE:
            return 2;

        case W_LANCE:
            return 3;

        default:
            return -1;
    }
}






struct DirectionStruct {
    int8_t df;
    int8_t dr;

    constexpr DirectionStruct(int8_t df, int8_t dr) : df(df), dr(dr) {}
    constexpr DirectionStruct() : DirectionStruct(0, 0) {}

    constexpr int8_t d_index() const { return dr*9 + df; }

    constexpr bool operator==(const DirectionStruct& other) const {
        return df == other.df && dr == other.dr;
    }
};
constexpr DirectionStruct NO_DIR = DirectionStruct(0, 0);
constexpr DirectionStruct NORTH_EAST = DirectionStruct(-1, -1);
constexpr DirectionStruct NORTH = DirectionStruct(0, -1);
constexpr DirectionStruct NORTH_WEST = DirectionStruct(1, -1);
constexpr DirectionStruct WEST = DirectionStruct(1, 0);
constexpr DirectionStruct SOUTH_WEST = DirectionStruct(1, 1);
constexpr DirectionStruct SOUTH = DirectionStruct(0, 1);
constexpr DirectionStruct SOUTH_EAST = DirectionStruct(-1, 1);
constexpr DirectionStruct EAST = DirectionStruct(-1, 0);

constexpr uint8_t NUM_1DIR = 8;
constexpr uint8_t MAX_SLIDING_DIRECTIONS = 4;

// max number of squares that can attack a given square
// 1 for each direction and 2 knights
constexpr uint8_t MAX_ATTACKERS = 10;

// operators to add/subtract direction from square
constexpr Square operator+(Square sq, DirectionStruct d) { return Square(int(sq) + d.d_index()); }
constexpr Square operator-(Square sq, DirectionStruct d) { return Square(int(sq) - d.d_index()); }
constexpr Square& operator+=(Square& sq, DirectionStruct d) { return sq = sq + d; }
constexpr Square& operator-=(Square& sq, DirectionStruct d) { return sq = sq - d; }

constexpr DirectionStruct operator*(int n, DirectionStruct d) {
    return DirectionStruct(n * d.df, n * d.dr);
}
constexpr DirectionStruct operator+(DirectionStruct d1, DirectionStruct d2) {
    return DirectionStruct(d1.df + d2.df, d1.dr + d2.dr);
}
constexpr DirectionStruct operator-(DirectionStruct d1, DirectionStruct d2) {
    return DirectionStruct(d1.df - d2.df, d1.dr - d2.dr);
}

// functions to convert between square, file and rank
constexpr Square make_square(File f, Rank r) { return Square(r*9 + f); };
constexpr File file_of(Square sq) { return File(sq % 9); };
constexpr Rank rank_of(Square sq) { return Rank(sq / 9); };

constexpr bool promotion_zone(Square sq, Color c) { 
    return rank_of(sq) <= R_3 && c == BLACK || rank_of(sq) >= R_7 && c == WHITE;
};

// safe way to add a direction to a square
constexpr Square add_direction(Square square, DirectionStruct d) {
    uint8_t f = file_of(square) + d.df;
    uint8_t r = rank_of(square) + d.dr;
    if (f > F_9 || r > R_9)
        return NO_SQUARE;
    else
        return square + d;
}

// enable increment and decrement operators
#define ENABLE_INCR_OPERATORS_ON(T) \
    constexpr T& operator++(T& t) { return t = T(int(t) + 1); } \
    constexpr T& operator--(T& t) { return t = T(int(t) - 1); } 

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(Color)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON


// class to represent a move
// a move is represented with 16 bits:
// 7 bits for either the from square or the dropped piece
// (the first 2 bits can be used to indicate if the move is a drop or not)
// (if bits 6 and 7 are both 1, then the move is a drop, there is no square that starts with 11)
// (THE PATENTED 6 7 METHOD(c))
// 7 bits for the to square
// 2 bits for flags. this is for now only used for promotion
class Move {
    public:
        Move() = default;
        constexpr explicit Move(uint16_t data) : data(data) {}

        constexpr Move(Square from, Square to, bool promotion = false) : 
            data((promotion ? 1u << 14 : 0u) | to << 7 | from) {}
        constexpr Move(PieceType dropped, Square to) :
            data(to << 7 | 0x60u | dropped) {}
            
        constexpr bool operator==(const Move& other) const { return data == other.data; }
            
        static constexpr Move null() { return Move(0); }
        constexpr bool is_null() const { return *this == null(); }

        constexpr bool is_drop() const { return (data & 0x60u) == 0x60u; }

        constexpr Square from() const {
            assert(!is_drop());
            return Square(data & 0x7Fu);
        }
        constexpr PieceType dropped() const {
            assert(is_drop());
            return PieceType(data & 0xFu);
        }
        constexpr Square to() const {
            return Square((data & 0x3F80u) >> 7);
        }
        constexpr bool is_promotion() const {
            return data & 0x4000u;
        }

        constexpr uint16_t raw() const { return data; }

        std::string to_string() const;

    protected:
        uint16_t data;
};

} // namespace harukashogi

#endif // TYPES_H