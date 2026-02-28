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
    std::string position, token;
    cmdStream >> token;

    // set the initial position
    if (token == "startpos")
        position = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
    else if (token == "sfen") {
        for (int i = 0; i < 3; ++i) {
            cmdStream >> token;
            position += token + " ";
        }
    }
    Position pos;
    pos.set(position);

    // make the moves if provided
    cmdStream >> token;
    if (token == "moves") {
        while (cmdStream >> token) {
            Move move = move_from_string(token);
            pos.make_move(move);
        }
    }

    // set the search manager's position
    searchManager.set_position(pos.sfen());
}


void USIEngine::go(std::istringstream& cmdStream) {
    std::string token;
    cmdStream >> token;

    SearchLimits limits;

    // blocking search for the given time limit and print bestmove
    if (token == "movetime") {
        cmdStream >> token;
        limits.moveTime = chr::milliseconds(std::stoi(token));
    }

    // starts searching. A stop command will be sent when the search is finished.
    else if (token == "infinite")
        limits.infinite = true;

    searchManager.set_limits(limits);
    searchManager.start_searching();
}


void USIEngine::stop() {
    searchManager.abort_search();
    searchManager.wait_search_finished();
}


} // namespace harukashogi