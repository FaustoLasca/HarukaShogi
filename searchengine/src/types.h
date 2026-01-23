#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

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
constexpr uint8_t NUM_SLIDING_TYPES = 3;
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
constexpr Piece make_piece(Color c, PieceType pt) { return Piece(c * NUM_PIECE_TYPES + pt + 1); };
constexpr Piece promote_piece( Piece p ) { return Piece(p + 6); };
constexpr Piece unpromote_piece( Piece p ) { return Piece(p - 6); };
constexpr bool is_promoted( PieceType pt ) { return pt >= P_SILVER; };
constexpr bool is_promoted( Piece p ) { return is_promoted(type_of(p)); };
// used to index the sliding move directions array
constexpr std::size_t sliding_type_index( PieceType pt ) { 
    switch (pt) {
        case BISHOP:
        case P_BISHOP:
            return 0;
        case ROOK:
        case P_ROOK:
            return 1;
        case LANCE:
            return 2;
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

struct Direction {
    int8_t df;
    int8_t dr;

    constexpr Direction(int8_t df, int8_t dr) : df(df), dr(dr) {}
    constexpr Direction() : Direction(0, 0) {}

    constexpr int8_t d_index() const { return dr*9 + df; }

    constexpr bool operator==(const Direction& other) const { return df == other.df && dr == other.dr; }
};
constexpr Direction NO_DIR = Direction(0, 0);
constexpr Direction NORTH_EAST = Direction(-1, -1);
constexpr Direction NORTH = Direction(0, -1);
constexpr Direction NORTH_WEST = Direction(1, -1);
constexpr Direction WEST = Direction(1, 0);
constexpr Direction SOUTH_WEST = Direction(1, 1);
constexpr Direction SOUTH = Direction(0, 1);
constexpr Direction SOUTH_EAST = Direction(-1, 1);
constexpr Direction EAST = Direction(-1, 0);

constexpr uint8_t NUM_DIRECTIONS = 8;
constexpr uint8_t MAX_SLIDING_DIRECTIONS = 4;

// max number of squares that can attack a given square
// 1 for each direction and 2 knights
constexpr uint8_t MAX_ATTACKERS = 10;

// operators to add/subtract direction from square
constexpr Square operator+(Square sq, Direction d) { return Square(int(sq) + d.d_index()); }
constexpr Square operator-(Square sq, Direction d) { return Square(int(sq) - d.d_index()); }
constexpr Square& operator+=(Square& sq, Direction d) { return sq = sq + d; }
constexpr Square& operator-=(Square& sq, Direction d) { return sq = sq - d; }

constexpr Direction operator*(int n, Direction d) { return Direction(n * d.df, n * d.dr); }
constexpr Direction operator+(Direction d1, Direction d2) { return Direction(d1.df + d2.df, d1.dr + d2.dr); }
constexpr Direction operator-(Direction d1, Direction d2) { return Direction(d1.df - d2.df, d1.dr - d2.dr); }

// functions to convert between square, file and rank
constexpr Square make_square(File f, Rank r) { return Square(r*9 + f); };
constexpr File file_of(Square sq) { return File(sq % 9); };
constexpr Rank rank_of(Square sq) { return Rank(sq / 9); };

constexpr bool promotion_zone(Square sq, Color c) { 
    return rank_of(sq) <= R_3 && c == BLACK || rank_of(sq) >= R_7 && c == WHITE;
};

// safe way to add a direction to a square
constexpr Square add_direction(Square square, Direction d) {
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
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON

struct Move {
    Square from = NO_SQUARE;
    Square to = NO_SQUARE;
    bool promotion = false;
    // type of piece involved in the move
    // for drops, this is the type of the piece dropped
    // for captures, this is the type of the captured piece
    // if the move is not a capture or drop, this is NO_PIECE_TYPE
    PieceType type_involved = NO_PIECE_TYPE;

    constexpr bool is_null() const { return from == NO_SQUARE && to == NO_SQUARE; };
    constexpr bool is_drop() const { return from == NO_SQUARE; };
};
constexpr Move NULL_MOVE = Move{NO_SQUARE, NO_SQUARE, false, NO_PIECE_TYPE};

} // namespace harukashogi

#endif // TYPES_H