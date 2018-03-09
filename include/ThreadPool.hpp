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
        : shouldRun{true}
    {
        createThreads(std::thread::hardware_concurrency());
    }

    ThreadPool(size_t size)
        : shouldRun{true}
    {
        createThreads(size);
    }

    ~ThreadPool()
    {
        shouldRun = false;
        cv.notify_all();
        for (auto& t : threads) {
            t.join();
        }
    }

    static ThreadPool& globalInstance()
    {
        static ThreadPool pool;
        return pool;
    }

    template <typename F, typename... Args>
    auto addTask(F&& fun, Args&&... args)
    {
        using ReturnType = typename std::result_of<F(Args...)>::type;
        Lock lock{mtx};
        if (buffer.size() + 1 > threads.size()) {
            threads.push_back(std::thread([this]() { run(); }));
        }
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
            // wait for new task or end of waiting on condition variable
            {
                Lock lock{mtx};
                cv.wait(lock, [&]() { return shouldRun == false || !buffer.empty(); });

                if (!buffer.empty()) {
                    task = std::move(buffer.front());
                    buffer.pop();
                }
            }

            // if there was task in buffer execute it and continue to next task
            if (task != nullptr) {
                task();
                task = nullptr;
                continue;
            }

            // otherwise stop looping
            return;
        }
    }

    std::atomic_bool shouldRun;
    std::vector<std::thread> threads;
    std::condition_variable cv;
    std::mutex mtx;
    TaskBuffer buffer;
};
} // namespace Simple

#endif /* ifndef SIMPLE_THREADPOOL_HPP */
