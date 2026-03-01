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
    threads.master().set_limits(limits);
    threads.start_searching();
}


void Engine::stop() {
    threads.abort_search();
    threads.wait_search_finished();
}


} // namespace harukashogi