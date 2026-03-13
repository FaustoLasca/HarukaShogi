#include "engine.h"
#include "misc.h"

namespace harukashogi {


void Engine::set_position(const std::string& sfen, const std::vector<std::string>& moves) {
    pos.set(sfen);
    for (const auto& move : moves) {
        pos.make_move(move_from_string(move));
    }

    for (auto& thread : threads) {
        thread->set_position(pos.sfen());
    }
}


void Engine::go(const SearchLimits& limits) {
    // wait for the previous search to finish
    // avoids race condition when returning immediately after ponderhit
    threads.wait_search_finished();

    threads.master().set_limits(limits);
    threads.master().set_stop(false);
    threads.master().set_ponderhit(false);
    
    threads.start_searching();
}


void Engine::stop() {
    threads.master().set_stop(true);
    threads.wait_search_finished();
}


void Engine::ponderhit() {
    threads.master().set_ponderhit(true);
}


} // namespace harukashogi