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

    class ThreadPool {
        using ThreadTask = std::function<void()>;
        using TaskBuffer = std::queue<ThreadTask>;

        using Lock = std::unique_lock<std::mutex>;

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
            shouldRun = false;
            cv.notify_all();
            for (auto &t : threads) {
                t.join();
            }
        }

        template <typename F, typename... Args>
        auto addTask(F &&fun, Args &&... args)
        {
            using ReturnType = typename std::result_of<F(Args...)>::type;
            Lock lock(mtx);
            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<F>(fun), std::forward<Args>(args)...));
            std::future<ReturnType> f = task->get_future();
            buffer.push([task]() { (*task)(); });
            cv.notify_all();
            return f;
        }

    private:
        void createThreads(size_t size)
        {
            for (size_t i = 0; i < size; ++i) {
                threads.push_back(std::thread([this]() { run(); }));
            }
        }

        void run()
        {
            ThreadTask task;

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
