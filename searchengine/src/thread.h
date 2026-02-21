#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace harukashogi {


class Thread {
    public:
        Thread(size_t id) : threadId(id), thread(&Thread::idle_loop, this) {}
        ~Thread();

        // the idle loop is the main loop of the thread
        // waits for the thread to be signaled to start searching
        void idle_loop();

        // non blocking functions to control the thread
        void start_searching();
        void abort_search();
        void exit();
        bool is_searching();
        // blocking function to wait for the thread to finish searching
        void wait_search_finished();

    protected:
        // the search function is the function that does the work of the thread, called inside the
        // idle loop.
        virtual void search() = 0;

        // checks if the searchFlag has been set to false.
        bool is_search_aborted() const;

        // used by the Worker, so needs to be protected.
        size_t threadId;

    private:
        std::thread thread;
        // mutex and condition variable to synchronize the thread
        std::mutex mutex;
        std::condition_variable cv;

        // flags to control the thread
        // search: set to true when the thread is signaled to start searching, set to false when 
        //         the thread is signaled to stop searching.
        // exit: when the flag is set, the thread will exit the idle loop.
        // searching: set to true when the thread is searching, set to false when the thread is not 
        //            searching. This is used to wait for the thread to finish searching.
        // 
        // read without holding the mutex, needs to be atomic.
        std::atomic<bool> searchFlag = false;
        // always read and written while holding the mutex, no need to be atomic.
        bool exitFlag = false;
        bool searchingFlag = false;
};


} // namespace harukashogi

#endif // THREAD_H