#pragma once
/**
 *  \brief     C++ thread pool
 *  \details   Supply a number of threads and distribute work to them.
 *  \author    Jakob Progsch, Václav Zeman
 *  \author    modified by Wolfram Rösler
 *  \date      2017-06-30
 *  \copyright Free for all - details on https://raw.githubusercontent.com/wolframroesler/ThreadPool/master/COPYING
 *
 * This version: https://github.com/wolframroesler/ThreadPool
 * Original version: https://github.com/progschj/ThreadPool
 */

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <algorithm>

 /**
  * Thread pool class.
  *
  * Example (in lieu of proper documentation):
  *
  *      // create thread pool with 4 worker threads
  *      ThreadPool pool(4);
  *
  *      // enqueue and store future
  *      auto result = pool([](int answer) { return answer; }, 42);
  *
  *      // get result from future
  *      std::cout << result.get() << std::endl;
  */
class ThreadPool {
public:
    // Ctor/dtor
    explicit ThreadPool(size_t threads);
    virtual ~ThreadPool();

    // Enqueue function
    template<class F, class... Args>
    auto AddTask(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>;

    // Obey the "rule of five"
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;

    // the task queue
    std::queue< std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads) {
    for (size_t i = 0; i < std::max(static_cast<size_t>(1), threads); ++i) {
        workers.emplace_back(
            [this] {
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
    }
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::AddTask(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop)
            std::cerr << "enqueue on stopped ThreadPool";

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers)
        worker.join();
}
