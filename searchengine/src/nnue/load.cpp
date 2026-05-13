#include "load.h"
#include "../types.h"
#include "../position.h"
#include "binpack.h"

#include <random>

namespace harukashogi {
namespace NNUE {


DataBatch::DataBatch(std::vector<DataSample>& samples) {
    batch_size = samples.size();

    // reserve memory for the vectors
    black_indexes.reserve(batch_size * ACTIVE_FEATURES);
    white_indexes.reserve(batch_size * ACTIVE_FEATURES);
    scores.reserve(batch_size);
    results.reserve(batch_size);
    stms.reserve(batch_size);

    // add the samples to the vectors
    for (const auto& sample : samples) {
        black_indexes.insert(black_indexes.end(),
                             sample.black_indexes.begin(),
                             sample.black_indexes.end());
        white_indexes.insert(white_indexes.end(),
                             sample.white_indexes.begin(),
                             sample.white_indexes.end());
        scores.push_back(sample.score);
        results.push_back(sample.result);
        stms.push_back(sample.stm);
    }
}


DataSample compute_sample(const Position& pos, int16_t score, Color winner, bool hflip) {
    DataSample sample;
    sample.score = score;
    sample.result = winner == NO_COLOR           ? 0.5f 
                  : winner == pos.side_to_move() ? 1.0f 
                  : 0.0f;
    sample.stm = pos.side_to_move() == BLACK ? 0.0f : 1.0f;

    Square bKingSq = hflip ? harukashogi::hflip(pos.king_square(BLACK)) 
                           : pos.king_square(BLACK);
    Square wKingSq = hflip ? harukashogi::hflip(pos.king_square(WHITE)) 
                           : pos.king_square(WHITE);

    size_t num_idxs = 0;
    Piece p;
    // add the board indexes
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if ((p = pos.piece(sq)) != NO_PIECE) {
            Square sq_flip = hflip ? harukashogi::hflip(sq) : sq;
            PieceType pt = type_of(p);
            Color c = color_of(p);
            sample.black_indexes[num_idxs] = FeatureSet::board_idx<BLACK>(bKingSq, c, pt, sq_flip);
            sample.white_indexes[num_idxs] = FeatureSet::board_idx<WHITE>(wKingSq, c, pt, sq_flip);
            ++num_idxs;
        }
    }
    // add the hand indexes
    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (int count = 0; count < pos.hand_count(c, pt); ++count) {
                sample.black_indexes[num_idxs] = FeatureSet::hand_idx<BLACK>(bKingSq, c, pt, count);
                sample.white_indexes[num_idxs] = FeatureSet::hand_idx<WHITE>(wKingSq, c, pt, count);
                ++num_idxs;
            }
        }
    }

    assert(num_idxs == ACTIVE_FEATURES);

    return sample;
}


std::shared_ptr<DataBatch> load_data_batch(
    const std::string& file_path,
    bool hflip,
    bool random_hflip
) {
    Position::init();
    std::mt19937 rng(std::random_device{}());

    std::vector<DataSample> samples;

    Binpack binpack(file_path, std::ios::in);
    GameData game;

    while (binpack.read_game(game)) {
        for (const auto& tuple : game.scoreMoves) {
            Move move = std::get<0>(tuple);
            int16_t score = std::get<1>(tuple);
            bool discard = std::get<2>(tuple);

            if (!discard) {
                if (random_hflip) hflip = rng() % 2 == 0;
                samples.push_back(compute_sample(game.pos, score, game.winner, hflip));
            }

            game.pos.make_move(move);
        }
        
    }

    return std::make_shared<DataBatch>(samples);
}


} // namespace NNUE
} // namespace harukashogi