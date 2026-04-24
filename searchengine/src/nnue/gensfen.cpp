#include "../engine.h"
#include "../search.h"
#include "../types.h"
#include "../movegen.h"
#include "../misc.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <filesystem>
#include <algorithm>

using namespace harukashogi;


class CVManager : public OutputManager {
    public:
        void on_best_move(Move bestMove, Move ponderMove) override {
            std::unique_lock<std::mutex> lock(mutex);
            this->bestMove = bestMove;
            is_ready = true;
            cv.notify_all();
        };
        void on_iter(const SearchInfo& info) override {
            this->score = info.eval;
        };

        Move wait_for_best_move() {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this] { return is_ready; });
            is_ready = false;
            Move m = bestMove;
            return m;
        }
        int get_score() { return score; }

    private:
        std::mutex mutex;
        std::condition_variable cv;
        bool is_ready = false;

        Move bestMove = Move::null();
        int score = 0;
};


struct DataPoint {
    std::string sfen;
    int score;
    float result;
};


int play_game(Engine& engine, CVManager& manager, OpeningBook& book, std::vector<DataPoint>& data){
    size_t gameStartIdx = data.size();

    std::mt19937 rng = std::mt19937(std::random_device{}());
    Position pos;
    pos.set();

    Move moveList[MAX_MOVES];
    Move move, *end;
    int numMoves = 0, score;

    // start the game with a random last move from the opening book
    while (!pos.is_game_over() && (move = book.sample_move(pos.get_key())) != Move::null()) {
        pos.make_move(move);
        numMoves++;

        // small chanche to exit the opening book early
        if (rng() % 50 == 0)
            break;
    }

    // play up to 5 random moves
    int nRandMoves = rng() % 5;
    for (int i = 0; i < nRandMoves && !pos.is_game_over(); i++) {
        end = generate<LEGAL>(pos, moveList);
        move = moveList[rng() % (end - moveList)];
        pos.make_move(move);
        numMoves++;
    }

    // main generation loop
    SearchLimits limits;
    while (!pos.is_game_over() && numMoves < 1000) {
        // search for 100ms and get the best move and score
        engine.set_position(pos.sfen());
        limits = SearchLimits();
        limits.moveTime = chr::milliseconds(100);
        engine.go(limits);
        move = manager.wait_for_best_move();
        score = manager.get_score();

        // check that the move is legal.
        // if it isn't something went wrong, choose a random legal move.
        end = generate<LEGAL>(pos, moveList);
        assert(end != moveList);
        if (std::find(moveList, end, move) == end) {
            std::cout << "-----------------------------------------------------------" << std::endl;
            std::cout << "ILLEGAL MOVE: " << move << std::endl;
            std::cout << "POSITION:     " << pos.sfen() << std::endl;
            std::cout << "-----------------------------------------------------------" << std::endl;
            move = moveList[rng() % (end - moveList)];
        }

        // filter out checks and positions where a capture is the best move
        // if a capture is the best move, we are likely to be in an unstable position
        if (!pos.checkers() && !pos.is_capture(move)) {
            data.push_back({pos.sfen(), score, pos.side_to_move() == BLACK ? 0.0f : 1.0f});
        }
        
        pos.make_move(move);
        numMoves++;
    }

    // if no new data points were added, return
    if (data.size() == gameStartIdx)
        return 0;

    Color winner = pos.get_winner();

    for (size_t i = gameStartIdx; i < data.size(); i++) {
        if (winner == NO_COLOR) {
            data[i].result = 0.5f;
        }
        else if ((data[i].result == 0 && winner == BLACK) || 
                 (data[i].result == 1 && winner == WHITE)) {
            data[i].result = 1.0f;
        }
        else {
            data[i].result = 0.0f;
        }
    }

    return data.size() - gameStartIdx;
}


void write_file(const std::string& filePath, std::vector<DataPoint>& data, int numPositions) {
    // write the data to a file
    std::ofstream file(filePath);
    assert(file.is_open());
    numPositions = std::min(numPositions, int(data.size()));
    for (size_t i=0; i<numPositions; i++)
        file << data[i].sfen << " | " << data[i].score << " | " << data[i].result << std::endl;
    file.close();

    // remove the data from the vector
    data.erase(data.begin(), data.begin() + numPositions);
}


void generate_data(const std::string& outDir, int totalPositions, int filePositions) {
    init();
    CVManager manager;
    Engine engine(manager);
    engine.resize_threadpool(1);
    engine.resize_tt(200);
    engine.set_own_book(false);
    OpeningBook book;

    std::vector<DataPoint> data;

    int generatedPositions = 0;
    int fileIdx = 0;
    while (generatedPositions < totalPositions) {
        generatedPositions += play_game(engine, manager, book, data);

        if (data.size() >= filePositions) {
            std::cout << "Writing file " << fileIdx << std::endl;
            write_file(outDir + "/" + std::to_string(fileIdx++) + ".txt", data, filePositions);
        }
    }
}


int main(int argc, char* argv[]) {
    assert(argc >= 2 && argc <= 4);
    std::string outDir = argv[1];
    int totalPositions = 1000000;
    if (argc >= 3) totalPositions = std::stoi(argv[2]);
    int filePositions = 1000;
    if (argc >= 4) filePositions = std::stoi(argv[3]);

    if (!std::filesystem::exists(outDir))
        std::filesystem::create_directories(outDir);

    std::cout << "Generating data in " << outDir << std::endl;
    std::cout << "Total positions:    " << totalPositions << std::endl;
    std::cout << "Positions per file: " << filePositions << std::endl;

    generate_data(outDir, totalPositions, filePositions);

    return 0;
};