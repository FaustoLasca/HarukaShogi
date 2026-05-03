#ifndef BINPACK_H
#define BINPACK_H

#include <ios>
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

#include "../position.h"

namespace harukashogi {
namespace NNUE {


struct GameData {
    Position pos;
    Color winner;
    std::vector<std::tuple<Move, int16_t, bool>> scoreMoves;
};


// class to pack the games into a binary .binp file
// the class opens a file when created and closes it when destroyed
class Binpack {
    public:
        Binpack(std::string filename, std::ios_base::openmode openMode);
        ~Binpack();

        // methods to write into a file
        void new_game(Position& pos);
        void add_move(Move move, int16_t score, bool discard);
        void game_over(Color winner);

        // methods to read from a file
        bool read_game(GameData& game);

    private:
        std::fstream file;
        std::vector<char> buffer;
        uint16_t count;
};


} // namespace NNUE
} // namespace harukashogi

#endif // BINPACK_H