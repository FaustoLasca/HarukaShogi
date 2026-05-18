#include "../position.h"
#include "../movegen.h"
#include "../engine.h"

#include <fstream>
#include <unordered_set>


using namespace harukashogi;


// hash function for the position bytes
// used to store the explored positions in a set to avoid duplicates
struct BytesPosHash {
    size_t operator()(const std::array<char, 45>& bytes) const {
        size_t hash = 4968795165;
        for (char b : bytes) {
            hash ^= b;
            hash *= 98765216219;
        }
        return hash;
    }
};


int main() {
    std::string gamesFileName = "/home/fausto/myProjects/HarukaShogi/data/games.txt";
    std::string startposFileName = "/home/fausto/myProjects/HarukaShogi/data/startpos.bin";

    std::ifstream file(gamesFileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << gamesFileName << "\n";
        return 1;
    }

    init();

    // read the games.txt file line by line
    // hold the explored positions in a set to avoid duplicates
    // then save the positions to a binary file
    std::unordered_set<std::array<char, 45>, BytesPosHash> positions;

    int kingSqCounts[NUM_SQUARES] = {0};

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        Position pos;
        pos.set();

        Move moveList[MAX_MOVES];
        Move move, *end;

        int moveCount = 0;
        while (ss >> token) {
            // discard the position if the game is over
            if (pos.is_game_over() || moveCount >= 100)
                break;

            // add the position to the set if it is not already in it
            std::array<char, 45> bytes;
            pos.to_bytes(bytes.data());
            if (positions.count(bytes) == 0) {
                positions.insert(bytes);
                Square bKingBucket = pos.king_square(BLACK);
                if (file_of(bKingBucket) > 4) bKingBucket = hflip(bKingBucket);
                Square wKingBucket = SQ_99 - pos.king_square(WHITE);
                if (file_of(wKingBucket) > 4) wKingBucket = hflip(wKingBucket);
                kingSqCounts[bKingBucket]++;
                kingSqCounts[wKingBucket]++;
            }
            

            // parse the move from the token
            move = move_from_string(token);
            // check that the move is legal
            end = generate<LEGAL>(pos, moveList);
            if (std::find(moveList, end, move) == end) {
                std::cout << "Invalid move: " << token << "\n";
                break;
            }
            // make the move
            pos.make_move(move);
            moveCount++;
        }
    }

    file.close();


    std::cout << "Found " << positions.size() << " unique positions\n";
    for (int i = 0; i < NUM_SQUARES; i++) {
        std::cout << "King square " << i << " count: " << kingSqCounts[i] << "\n";
    }


    // save the positions to a binary file
    // simply as a sequence of bytes
    std::ofstream outFile(startposFileName, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open " << startposFileName << "\n";
        return 1;
    }
    for (const auto& bytes : positions) {
        outFile.write(bytes.data(), 45);
    }
    outFile.close();

    std::cout << "Saved " << positions.size() << " positions to " << startposFileName << "\n";
}