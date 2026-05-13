#include <iostream>
#include <random>

#include "position.h"
#include "movegen.h"
#include "nnue/nnue.h"
#include "types.h"
#include "nnue/binpack.h"

using namespace harukashogi;


bool accumulators_match(const NNUE::AccumulatorType& a, const NNUE::AccumulatorType& b) {
    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (size_t i = 0; i < NNUE::ACCUMULATOR_SIZE; ++i) {
            if (a[c][i] != b[c][i]) {
                std::cout << "Mismatch in " << (c == BLACK ? "BLACK" : "WHITE")
                          << " accumulator at index " << i << ": "
                          << a[c][i] << " != " << b[c][i] << std::endl;
                return false;
            }
        }
    }
    return true;
}


int main() {
    // Position::init();

    // Position pos;
    // pos.set();

    // NNUE::NNUE nnue;
    // NNUE::AccumulatorType acc;
    // NNUE::AccumulatorType updatedAcc;
    // NNUE::AccumulatorType recomputedAcc;

    // nnue.feature_transformer().compute(pos, acc);

    // std::mt19937 rng(0);
    // int testedMoves = 0;
    // constexpr int MaxPlies = 300;

    // for (int ply = 0; ply < MaxPlies && !pos.is_game_over(); ++ply) {
    //     Move moves[MAX_MOVES];
    //     Move* end = generate<LEGAL>(pos, moves);

    //     if (moves == end) {
    //         break;
    //     }

    //     for (Move* m = moves; m < end; ++m) {
    //         MoveDiff diff = pos.make_move(*m);

    //         if (NNUE::requires_recompute(diff)) {
    //             nnue.feature_transformer().compute(pos, updatedAcc);
    //         }
    //         else {
    //             nnue.feature_transformer().incremental_update(
    //                 pos.king_square(), diff, acc, updatedAcc
    //             );
    //         }
    //         nnue.feature_transformer().compute(pos, recomputedAcc);

    //         if (!accumulators_match(updatedAcc, recomputedAcc)) {
    //             std::cout << "Failed after move " << m->to_string()
    //                       << " at ply " << ply << std::endl;
    //             return 1;
    //         }

    //         pos.unmake_move(*m);
    //         ++testedMoves;
    //     }

    //     Move move = moves[rng() % (end - moves)];
    //     MoveDiff diff = pos.make_move(move);
    //     if (NNUE::requires_recompute(diff)) {
    //         nnue.feature_transformer().compute(pos, acc);
    //     }
    //     else {
    //         nnue.feature_transformer().incremental_update(
    //             pos.king_square(), diff, acc, updatedAcc
    //         );
    //         acc = updatedAcc;
    //     }
    // }

    // std::cout << "Accumulator update test passed for " << testedMoves
    //           << " legal moves." << std::endl;

    Position::init();

    Position pos;
    pos.set();

    NNUE::NNUE nnue;
    NNUE::AccumulatorType acc;

    NNUE::Binpack binpack("data/test_binps/0.binp", std::ios::in);
    NNUE::GameData game;

    binpack.read_game(game);
    size_t idx = 0;
    for (int i = 0; i < 10; i++) {
        nnue.feature_transformer().compute(game.pos, acc);
        std::cout << nnue.evaluate(acc, game.pos.side_to_move()) << std::endl;
        do {
            game.pos.make_move(std::get<0>(game.scoreMoves[idx]));
            idx++;
        } while(std::get<2>(game.scoreMoves[idx]));
    }

    return 0;
}