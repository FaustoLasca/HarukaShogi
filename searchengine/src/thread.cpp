#include <atomic>
#include <cassert>

#include "thread.h"

namespace harukashogi {


Thread::~Thread() {
    exit();
    thread.join();
}


void Thread::idle_loop() {
    while (true) {
        // wait for the thread to be signaled to start searching or to exit
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return searchFlag || exitFlag; });

        if (exitFlag)
            break;

        // set the searching flag to true and unlock the mutex
        searchingFlag = true;
        lock.unlock();

        search();

        // when the search is finished, set the searching flag to false and loop
        lock.lock();
        // in case the search finished before the thread was signaled to stop searching, set the
        // search flag to false
        searchFlag = false;
        searchingFlag = false;
        cv.notify_all();
    }
}


void Thread::start_searching() {
    // set the search flag to true and notify the thread
    std::unique_lock<std::mutex> lock(mutex);
    assert(!searchingFlag);
    searchFlag = true;
    cv.notify_all();
}


void Thread::abort_search() {
    // if the thread is searching, set the search flag to false and notify the thread
    std::unique_lock<std::mutex> lock(mutex);
    if (searchingFlag)
        searchFlag = false;
}


void Thread::exit() {
    // if the thread is searching, abort the search
    abort_search();
    wait_search_finished();
    // set the exit flag to true and notify the thread
    std::unique_lock<std::mutex> lock(mutex);
    exitFlag = true;
    cv.notify_all();
}


void Thread::wait_search_finished() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return !searchingFlag && !searchFlag; });
}


bool Thread::is_search_aborted() const {
    assert(searchingFlag);
    return !searchFlag.load(std::memory_order_relaxed);
}


} // namespace harukashogi