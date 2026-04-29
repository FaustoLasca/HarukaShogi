#include "binpack.h"
#include "../evaluate.h"

#include <cstring>

namespace harukashogi {
namespace NNUE {


Binpack::Binpack(std::string filename, std::ios_base::openmode openMode) : 
    file(filename, openMode | std::ios::binary) 
{
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    if (openMode == std::ios::out)
        file.write("HARUKA", 6);
    else {
        char header[6];
        file.read(header, 6);
        if (std::string(header) != "HARUKA") {
            throw std::runtime_error("Invalid header in file: " + filename);
        }
    }
}


Binpack::~Binpack() {
    file.close();
}


void Binpack::new_game(Position& pos) {
    // initializes the beffer with the incomplete stem
    // the first 45 bytes contain the position, result+count isn't initialized yet
    count = 0;
    buffer.resize(47);
    pos.to_bytes(buffer.data());
}


void Binpack::add_move(Move move, int16_t score, bool discard) {
    count++;
    size_t start = buffer.size();
    buffer.resize(start + 4);
    // put the move into the buffer
    std::memcpy(&buffer[start], &move, 2);
    if (discard)
        score = INF_SCORE;
    // put the score into the buffer
    std::memcpy(&buffer[start + 2], &score, 2);
    // increment the count
}


void Binpack::game_over(Color winner) {
    // if no moves were added, don't write anything
    if (count == 0)
        return;
    // set the winner and count that were uninitialized
    count |= (uint16_t)winner << 14;
    std::memcpy(&buffer[45], &count, 2);

    file.write(buffer.data(), buffer.size());

    buffer.clear();
    count = 0;
}


bool Binpack::read_game(GameData& game) {
    uint16_t move;
    uint16_t score;
    char stem[47];
    // read the stem from the file
    if (file.read(stem, 47)) {
        // set the position
        game.scoreMoves.clear();
        game.pos.from_bytes(stem);
        // set the winner and movecount
        game.winner = Color(*(uint16_t*)(stem+45) >> 14);
        count = *(uint16_t*)(stem+45) & 0x3FFF;

        // read the moves and scores and add them to the game data
        for (int i = 0; i < count; ++i) {
            file.read((char*)&move, 2);
            file.read((char*)&score, 2);
            game.scoreMoves.push_back({Move(move), score, score == INF_SCORE});
        }

        // reset the buffer
        buffer.clear();
        count = 0;

        return true;
    }

    // if the read failed, return false
    return false;
}


} // namespace NNUE
} // namespace harukashogi