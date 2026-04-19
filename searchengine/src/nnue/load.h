#ifndef LOADER_H
#define LOADER_H

#include <vector>
#include <array>
#include <memory>

#include "nnue.h"

namespace harukashogi {
namespace NNUE {


constexpr size_t ACTIVE_FEATURES = 40;


struct DataSample {
    std::array<int, ACTIVE_FEATURES> black_indexes;
    std::array<int, ACTIVE_FEATURES> white_indexes;
    float score;
    float result;
    float stm;
};


struct DataBatch {
    DataBatch(std::vector<DataSample>& samples);

    size_t batch_size = 0;

    std::vector<int> black_indexes;
    std::vector<int> white_indexes;
    std::vector<float> scores;
    std::vector<float> results;
    std::vector<float> stms;
};


DataSample compute_sample(std::string sfen, float score, float result);
std::shared_ptr<DataBatch> load_data_batch(const std::string& file_path);


} // namespace NNUE
} // namespace harukashogi

#endif // LOADER_H