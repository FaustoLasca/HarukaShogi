#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>

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
        bool is_master() const { return threadId == 0; }

    private:
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

        std::thread thread;
};


// template <typename T>
// concept ThreadType = std::derived_from<T, Thread>;


template <typename T>
class ThreadPool {
    static_assert(std::derived_from<T, Thread>, "T must derive from Thread");

    public:
        // C++ black magic to pass arguments to the constructor of the threads in the pool
        template <typename... Args>
        ThreadPool(size_t numThreads, Args&&... args) {
            for (size_t i = 0; i < numThreads; i++)
                threads.push_back(std::make_unique<T>(i, std::forward<Args>(args)...));
        }
        
        // same functions as the Thread class, but for the entire thread pool
        void start_searching() { master().start_searching(); }
        // function called by the master thread to start the searching of the slaves
        void slaves_start_searching() {
            for (size_t i = 1; i < threads.size(); i++)
                threads[i]->start_searching();
        }

        void abort_search() { for (auto& thread : threads) thread->abort_search(); }
        void exit() { for (auto& thread : threads) thread->exit(); }
        bool is_searching() {
            for (auto& thread : threads)
                if (thread->is_searching()) return true;
            return false;
        }
        void wait_search_finished() { for (auto& thread : threads) thread->wait_search_finished(); }
        void wait_search_finished_slaves() {
            for (size_t i = 1; i < threads.size(); i++)
                threads[i]->wait_search_finished();
        }

        T& master() { return *threads[0]; } // returns the master thread

        // operator to access the threads in the pool
        T& operator[](size_t index) { return *threads[index]; }
        size_t size() const { return threads.size(); }
        auto begin() { return threads.begin(); }
        auto end() { return threads.end(); }

    private:
        std::vector<std::unique_ptr<T>> threads;
};


} // namespace harukashogi

#endif // THREAD_H