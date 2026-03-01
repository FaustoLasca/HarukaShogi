#include "usi.h"
#include "misc.h"

namespace harukashogi {


void USIEngine::run() {
    std::string command, token;
    do {
        std::getline(std::cin, command);
        std::istringstream cmdStream(command);

        token.clear();
        cmdStream >> std::skipws >> token;

        if (token == "usi")
            usi();

        // else if (token == "debug")
        //     debug(cmdStream);

        else if (token == "isready")
            isready();

        // else if (token == "setoption")
        //     setoption(cmdStream);

        // else if (token == "ucinewgame")
        //     usinewgame();
        
        else if (token == "position")
            position(cmdStream);

        else if (token == "go")
            go(cmdStream);

        else if (token == "stop")
            stop();

        // else if (token == "ponderhit")
        //     ponderhit();

        // else if (token == "gameover")
        //     gameover(cmdStream);

        // unknown commands are ignored, as per the USI protocol

    } while (token != "quit");
}


void USIEngine::usi() {
    std::cout << "id name Haruka Shogi\n"
              << "id author Fausto Lasca" << std::endl;

    // TODO: add options

    std::cout << "usiok" << std::endl;
}


void USIEngine::isready() {
    std::cout << "readyok" << std::endl;
}


void USIEngine::position(std::istringstream& cmdStream) {
    std::string sfen, token;
    std::vector<std::string> moves;

    cmdStream >> token;

    // set the initial position
    if (token == "startpos")
        sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
    else if (token == "sfen") {
        for (int i = 0; i < 3; ++i) {
            cmdStream >> token;
            sfen += token + " ";
        }
    }

    // make the moves if provided
    cmdStream >> token;
    if (token == "moves") {
        while (cmdStream >> token) {
            moves.push_back(token);
        }
    }

    engine.set_position(sfen, moves);
}


void USIEngine::go(std::istringstream& cmdStream) {
    std::string token;

    SearchLimits limits;

    while (cmdStream >> token) {
        if (token == "movetime") {
            cmdStream >> token;
            limits.moveTime = chr::milliseconds(std::stoi(token));
        }
        if (token == "infinite")
            limits.infinite = true;

        if (token == "ponder")
            limits.ponder = true;

        if (token == "btime") {
            cmdStream >> token;
            limits.time[BLACK] = chr::milliseconds(std::stoi(token));
        }
        if (token == "wtime") {
            cmdStream >> token;
            limits.time[WHITE] = chr::milliseconds(std::stoi(token));
        }
        if (token == "binc") {
            cmdStream >> token;
            limits.inc[BLACK] = chr::milliseconds(std::stoi(token));
        }
        if (token == "winc") {
            cmdStream >> token;
            limits.inc[WHITE] = chr::milliseconds(std::stoi(token));
        }
        if (token == "byoyomi") {
            cmdStream >> token;
            limits.byoyomi = chr::milliseconds(std::stoi(token));
        }
        if (token == "depth") {
            cmdStream >> token;
            limits.depth = std::stoi(token);
        }
        if (token == "nodes") {
            cmdStream >> token;
            limits.nodes = std::stoull(token);
        }
        if (token == "movestogo") {
            cmdStream >> token;
            limits.movesToGo = std::stoi(token);
        }
    }

    engine.go(limits);
}


void USIEngine::stop() {
    engine.stop();
}


void USIManager::on_best_move(Move bestMove, Move ponderMove) {
    std::cout << "bestmove " << bestMove << std::endl;
}

void USIManager::on_iter(const SearchInfo& info) {
    auto time = chr::duration_cast<chr::milliseconds>(
        chr::steady_clock::now() - info.startTime
    );
    // avoid division by zero
    long elapsed = std::max(time.count(), long(1));
    long nps = info.nodeCount * long(1000) / elapsed;

    std::cout << "info "
              << "depth " << info.depth << " "
              << "score cp " << info.eval << " "
              << "time "  << time.count() << " "
              << "nodes " << info.nodeCount << " "
              << "nps " << nps << std::endl;
}


} // namespace harukashogi