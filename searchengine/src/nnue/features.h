#ifndef FEATURES_H
#define FEATURES_H

#include "../types.h"

namespace harukashogi {
namespace NNUE {

    
constexpr size_t BoardFeatures = size_t(NUM_PIECES) * size_t(NUM_SQUARES);
constexpr size_t HandFeatures = 2 * 38; // all pieces but the kings in hand

#define BI(i) (i * size_t(NUM_SQUARES))
// contains the starting index for each board piece, before adding the bucket index
constexpr size_t BoardIdx[NUM_PIECES+1] = {
    0, // NO_PIECE
    // black pieces
    BI(0),  BI(1),  BI(2),  BI(3),  BI(4),  BI(5),  BI(6),  BI(7),
    BI(8),  BI(9),  BI(10), BI(11), BI(12), BI(13),
    // white pieces
    BI(14), BI(15), BI(16), BI(17), BI(18), BI(19), BI(20), BI(21),
    BI(22), BI(23), BI(24), BI(25), BI(26), BI(27),
};
#undef BI

// contains the starting index for each hand piece, before adding the bucket index
constexpr size_t HandIdx[NUM_UNPROMOTED_PIECE_TYPES] = {
    0, // king

    BoardFeatures + 0, // gold
    BoardFeatures + 4, // silver
    BoardFeatures + 8, // lance
    BoardFeatures + 12, // knight
    BoardFeatures + 16, // bishop
    BoardFeatures + 18, // rook
    BoardFeatures + 20, // pawn
};


struct P {
    static constexpr size_t NumFeatures = BoardFeatures + HandFeatures;

    template <Color perspective>
    static inline size_t board_idx(Square kingSq, Color c, PieceType pt, Square sq) {
        if constexpr (perspective == WHITE) {
            c = ~c;
            sq = SQ_99 - sq;
        }
    
        return BoardIdx[make_piece(c, pt)] + sq;
    }


    template <Color perspective>
    static inline size_t hand_idx(Square kingSq, Color c, PieceType pt, int count) {
        if constexpr (perspective == WHITE) {
            c = ~c;
        }
        
        return HandIdx[pt] + count + (c == BLACK ? 0 : 38);
    }


    template <Color perspective>
    static inline bool requires_recompute(MoveDiff diff) {
        // no recomputation needed for P features
        return false;
    }
};


struct KB9 {
    static constexpr size_t NumBuckets = 9;
    static constexpr size_t NumFeatures = NumBuckets * (BoardFeatures + HandFeatures);

    static constexpr size_t BDelta = BoardFeatures + HandFeatures;
    #define B(i) (i * BDelta)
    // contains the starting index for each bucket
    static constexpr size_t BucketIdx[81] = {
        B(0), B(0), B(0), B(1), B(1), B(1), B(2), B(2), B(2),
        B(0), B(0), B(0), B(1), B(1), B(1), B(2), B(2), B(2),
        B(0), B(0), B(0), B(1), B(1), B(1), B(2), B(2), B(2),
        B(3), B(3), B(3), B(4), B(4), B(4), B(5), B(5), B(5),
        B(3), B(3), B(3), B(4), B(4), B(4), B(5), B(5), B(5),
        B(3), B(3), B(3), B(4), B(4), B(4), B(5), B(5), B(5),
        B(6), B(6), B(6), B(7), B(7), B(7), B(8), B(8), B(8),
        B(6), B(6), B(6), B(7), B(7), B(7), B(8), B(8), B(8),
        B(6), B(6), B(6), B(7), B(7), B(7), B(8), B(8), B(8),
    };
    #undef B


    template <Color perspective>
    static inline size_t board_idx(Square kingSq, Color c, PieceType pt, Square sq) {
        if constexpr (perspective == WHITE) {
            kingSq = SQ_99 - kingSq;
            c = ~c;
            sq = SQ_99 - sq;
        }
    
        return BucketIdx[kingSq] + BoardIdx[make_piece(c, pt)] + sq;
    }


    template <Color perspective>
    static inline size_t hand_idx(Square kingSq, Color c, PieceType pt, int count) {
        if constexpr (perspective == WHITE) {
            kingSq = SQ_99 - kingSq;
            c = ~c;
        }
        
        return BucketIdx[kingSq] + HandIdx[pt] + count + (c == BLACK ? 0 : 38);
    }


    template <Color perspective>
    static inline bool requires_recompute(MoveDiff diff) {
        // only recompute if the stm's king changes bucket
        if (diff.fromPt == KING) {
            return BucketIdx[diff.toSq] != BucketIdx[diff.fromSq];
        }
        return false;
    }
};


} // namespace NNUE
} // namespace harukashogi

#endif // FEATURES_H