#ifndef FEATURES_H
#define FEATURES_H

#include "../types.h"

namespace harukashogi {
namespace NNUE {


// Promoted pieces and gold are treated as the same pieces
constexpr size_t BoardFeatures = 20 * size_t(NUM_SQUARES);
constexpr size_t HandFeatures = 2 * 38; // all pieces but the kings in hand

#define BI(i) (i * size_t(NUM_SQUARES))
// contains the starting index for each board piece, before adding the bucket index
constexpr size_t BoardIdx[NUM_PIECES+1] = {
    0, // NO_PIECE
    // black pieces
    BI(0),  BI(1),  BI(2),  BI(3),  BI(4),  BI(5),  BI(6),  BI(7),
    BI(1),  BI(1),  BI(1), BI(8), BI(9), BI(1),
    // white pieces
    BI(10), BI(11), BI(12), BI(13), BI(14), BI(15), BI(16), BI(17),
    BI(11), BI(11), BI(11), BI(18), BI(19), BI(11),
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


struct HalfKA {
    static constexpr size_t NumBuckets = 45;
    static constexpr size_t NumFeatures = NumBuckets * (BoardFeatures + HandFeatures);

    static constexpr size_t BDelta = BoardFeatures + HandFeatures;
    #define B(i) (i * BDelta)
    // contains the starting index for each bucket
    static constexpr size_t BucketIdx[81] = {
        B(44), B(43), B(42), B(41), B(40), B(41), B(42), B(43), B(44),
        B(39), B(38), B(37), B(36), B(35), B(36), B(37), B(38), B(39),
        B(34), B(33), B(32), B(31), B(30), B(31), B(32), B(33), B(34),
        B(29), B(28), B(27), B(26), B(25), B(26), B(27), B(28), B(29),
        B(24), B(23), B(22), B(21), B(20), B(21), B(22), B(23), B(24),
        B(19), B(18), B(17), B(16), B(15), B(16), B(17), B(18), B(19),
        B(14), B(13), B(12), B(11), B(10), B(11), B(12), B(13), B(14),
        B( 9), B( 8), B( 7), B( 6), B( 5), B( 6), B( 7), B( 8), B( 9),
        B( 4), B( 3), B( 2), B( 1), B( 0), B( 1), B( 2), B( 3), B( 4),
    };
    #undef B


    template <Color perspective>
    static inline size_t board_idx(Square kingSq, Color c, PieceType pt, Square sq) {
        if constexpr (perspective == WHITE) {
            kingSq = SQ_99 - kingSq;
            c = ~c;
            sq = SQ_99 - sq;
        }

        // if the king is on the right side of the board, flip the board horizontally
        if (file_of(kingSq) > 4) {
            sq = hflip(sq);
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
        // only recompute if the stm's king moves
        if (diff.fromPt == KING && diff.stm == perspective) {
            return true;
        }
        return false;
    }
};


struct R12KB {
    static constexpr size_t NumBuckets = 11;
    static constexpr size_t NumFeatures = NumBuckets * (BoardFeatures + HandFeatures);

    static constexpr size_t BDelta = BoardFeatures + HandFeatures;
    #define B(i) (i * BDelta)
    // contains the starting index for each bucket
    static constexpr size_t BucketIdx[81] = {
        B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10),
        B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10),
        B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10),
        B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10),
        B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10),
        B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10),
        B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10), B(10),
        B( 9), B( 8), B( 7), B( 6), B( 5), B( 6), B( 7), B( 8), B( 9),
        B( 4), B( 3), B( 2), B( 1), B( 0), B( 1), B( 2), B( 3), B( 4),
    };
    #undef B

    static constexpr bool FlipSq[81] = {
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1,
    };


    template <Color perspective>
    static inline size_t board_idx(Square kingSq, Color c, PieceType pt, Square sq) {
        if constexpr (perspective == WHITE) {
            kingSq = SQ_99 - kingSq;
            c = ~c;
            sq = SQ_99 - sq;
        }

        // if the king is on the right side of the board, flip the board horizontally
        if (FlipSq[kingSq]) {
            sq = hflip(sq);
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
        // only recompute if the stm's king moves
        if (diff.fromPt == KING && diff.stm == perspective) {
            if constexpr (perspective == WHITE) {
                diff.fromSq = SQ_99 - diff.fromSq;
                diff.toSq = SQ_99 - diff.toSq;
            }

            return BucketIdx[diff.fromSq] != BucketIdx[diff.toSq]
                ||    FlipSq[diff.fromSq] !=    FlipSq[diff.toSq];
        }
        return false;
    }
};


} // namespace NNUE
} // namespace harukashogi

#endif // FEATURES_H