#include "../engine.h"
#include "../search.h"
#include "../types.h"
#include "../movegen.h"
#include "../misc.h"
#include "../nnue/binpack.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <algorithm>
#include <random>

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


class Startpos {
    public:
        Startpos(const std::string& fileName) {
            std::ifstream file(fileName, std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + fileName);
            }

            std::array<char, 45> buffer;
            while (file.read(buffer.data(), 45)) {
                bytes.push_back(buffer);
            }

            rng = std::mt19937(std::random_device{}());
        }

        void sample(Position& pos) {
            pos.from_bytes(bytes[rng() % bytes.size()].data());
        }

    private:
        std::vector<std::array<char, 45>> bytes;
        std::mt19937 rng;
};


int play_game(Engine& engine, CVManager& manager, Startpos& startpos, 
              NNUE::Binpack& binpack, std::mt19937& rng){
    // sample a random startpos
    Position pos;
    startpos.sample(pos);

    std::cout << pos.sfen() << std::endl;

    Move moveList[MAX_MOVES];
    Move move, *end;
    int numMoves = 0, validMoves = 0, randomMoves = 0, score;

    // play up to 3 random moves
    int nRandMoves = rng() % 3;
    for (int i = 0; i < nRandMoves && !pos.is_game_over(); i++) {
        end = generate<LEGAL>(pos, moveList);
        move = moveList[rng() % (end - moveList)];
        pos.make_move(move);
        numMoves++;
        randomMoves++;
    }

    // start searching from the current position
    binpack.new_game(pos);

    int ply = 0;

    // main generation loop
    SearchLimits limits;
    while (!pos.is_game_over() && numMoves < 1000) {
        // search for 100ms and get the best move and score
        engine.set_position(pos.sfen());
        limits = SearchLimits();
        limits.depth = 3;
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
        bool discard = pos.is_capture(move) || pos.checkers() || score > 20000 || score < -20000;

        // randomly change the move for a random move with low probability in the first 16 plies
        if (ply < 16 && randomMoves < 6 && rng() % 8 == 0) {
            move = moveList[rng() % (end - moveList)];
            randomMoves++;
        }

        binpack.add_move(move, score, discard);
        if (!discard) {
            validMoves++;
        }
        
        pos.make_move(move);
        numMoves++;
    }

    Color winner = pos.get_winner();

    binpack.game_over(winner);

    return validMoves;
}


void generate_data(const std::string& outDir, int totalPositions, int filePositions) {
    init();
    CVManager manager;
    Engine engine(manager);
    engine.resize_threadpool(1);
    engine.resize_tt(200);
    engine.set_own_book(false);

    Startpos startpos("/home/fausto/myProjects/HarukaShogi/data/startpos.bin");

    std::mt19937 rng(std::random_device{}());

    int generatedPositions = 0;
    for (int fileIdx = 0; generatedPositions < totalPositions; fileIdx++) {
        NNUE::Binpack binpack(outDir + "/" + std::to_string(fileIdx) + ".binp", std::ios::out);
        for (int count = 0; count < filePositions; ) {
            char posBytes[45];
            int numMoves = play_game(engine, manager, startpos, binpack, rng);
            count += numMoves;
            generatedPositions += numMoves;
        }

        std::cout << "Generated file " << fileIdx << std::endl;
    }
}


int main(int argc, char* argv[]) {
    assert(argc >= 2 && argc <= 4);
    std::string outDir = argv[1];
    int totalPositions = 10000000;
    if (argc >= 3) totalPositions = std::stoi(argv[2]);
    int filePositions = 100000;
    if (argc >= 4) filePositions = std::stoi(argv[3]);

    if (!std::filesystem::exists(outDir))
        std::filesystem::create_directories(outDir);

    std::cout << "Generating data in " << outDir << std::endl;
    std::cout << "Total positions:    " << totalPositions << std::endl;
    std::cout << "Positions per file: " << filePositions << std::endl;

    generate_data(outDir, totalPositions, filePositions);

    return 0;
};