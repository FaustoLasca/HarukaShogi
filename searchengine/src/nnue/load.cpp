#include "load.h"
#include "../types.h"
#include "../position.h"

#include <fstream>
#include <sstream>
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


DataSample compute_sample(std::string sfen, float score, float result, bool hflip) {
    Position pos;
    pos.set(sfen);

    DataSample sample;
    sample.score = score;
    sample.result = result;
    sample.stm = pos.side_to_move() == BLACK ? 0.0f : 1.0f;

    size_t num_idxs = 0;
    Piece p;
    // add the board indexes
    for (Square sq = SQ_11; sq < NUM_SQUARES; ++sq) {
        if ((p = pos.piece(sq)) != NO_PIECE) {
            Square sq_flip = hflip ? harukashogi::hflip(sq) : sq;
            PieceType pt = type_of(p);
            Color c = color_of(p);
            sample.black_indexes[num_idxs] = NNUE::board_idx<BLACK>(c, pt, sq_flip);
            sample.white_indexes[num_idxs] = NNUE::board_idx<WHITE>(c, pt, sq_flip);
            ++num_idxs;
        }
    }
    // add the hand indexes
    for (Color c = BLACK; c < NUM_COLORS; ++c) {
        for (PieceType pt = GOLD; pt < NUM_UNPROMOTED_PIECE_TYPES; ++pt) {
            for (int count = 0; count < pos.hand_count(c, pt); ++count) {
                sample.black_indexes[num_idxs] = NNUE::hand_idx<BLACK>(c, pt, count);
                sample.white_indexes[num_idxs] = NNUE::hand_idx<WHITE>(c, pt, count);
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

    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    std::vector<DataSample> samples;
    std::string line, sfen;
    float score, result;

    while (std::getline(file, line)) {
        size_t p1 = line.find("|");
        size_t p2 = line.find("|", p1 + 1);

        sfen = line.substr(0, p1);
        score = std::stof(line.substr(p1 + 1, p2 - p1 - 1));
        result = std::stof(line.substr(p2 + 1, line.size() - p2 - 1));

        if (random_hflip) hflip = rng() % 2 == 0;
        samples.push_back(compute_sample(sfen, score, result, hflip));
    }

    return std::make_shared<DataBatch>(samples);
}


} // namespace NNUE
} // namespace harukashogi