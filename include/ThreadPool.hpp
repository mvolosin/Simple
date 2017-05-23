#ifndef SIMPLE_THREADPOOL_HPP
#define SIMPLE_THREADPOOL_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <vector>

namespace Simple {

    template <class R>
    class ThreadPool;

    template <class R, class... Args>
    class ThreadPool<R(Args...)> {
        using Task = std::packaged_task<R(Args...)>;
        using Lock = std::unique_lock<std::mutex>;
        using TaskBuffer = std::queue<Task>;

    public:
        ThreadPool()
        {
            createThreads(4);
        }

        ThreadPool(size_t size)
        {
            createThreads(size);
        }

        ~ThreadPool()
        {
            {
                Lock lock(mtx);
                shouldRun = false;
                cv.notify_all();
            }
            for (auto &t : threads) {
                t.join();
            }
        }

        std::future<R> addTask(std::function<R(Args...)> &&fun)
        {
            Lock lock(mtx);
            Task task(fun);
            auto f = task.get_future();
            buffer.push(std::move(task));
            cv.notify_all();
            return f;
        }

    private:
        void createThreads(size_t size)
        {
            for (size_t i = 0; i < size; ++i) {
                threads.push_back(std::move(std::thread([this]() { run(); })));
            }
        }

        void run()
        {
            Task task;

            while (true) {
                {
                    Lock lock(mtx);

                    if (!shouldRun)
                        break;

                    // if buffer is empty
                    if (buffer.empty()) {
                        cv.wait(lock);
                    }

                    // if buffer is still empty (spurious wakeup or destructor called)
                    if (buffer.empty())
                        continue;

                    task = std::move(buffer.front());
                    buffer.pop();
                }

                // execute task
                task();
            }
        }

        std::atomic_bool shouldRun = true;
        std::vector<std::thread> threads;
        std::condition_variable cv;
        std::mutex mtx;
        TaskBuffer buffer;
    };
}

#endif /* ifndef SIMPLE_THREADPOOL_HPP */
