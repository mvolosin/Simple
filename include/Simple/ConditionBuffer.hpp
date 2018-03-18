#ifndef SIMPLE_CONDITION_BUFFER_HPP
#define SIMPLE_CONDITION_BUFFER_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace Simple {
template <typename T, size_t size>
class ConditionBuffer {
    using MutexType = std::mutex;
    using LockType = std::unique_lock<MutexType>;

    size_t maxBufferSize;
    std::atomic_bool enabled;
    MutexType mutex;
    std::condition_variable cv;
    std::queue<T> queue;

public:
    ConditionBuffer()
        : maxBufferSize{size}
        , enabled{true} {};

    void disable()
    {
        enabled = false;
        cv.notify_all();
    }

    void enable()
    {
        enabled = true;
    }

    bool put(T&& element)
    {
        {
            LockType lock{mutex};
            if (queue.size() == maxBufferSize)
                return false;
            queue.push(std::forward<T>(element));
            cv.notify_all();
        }
        return true;
    }

    bool get(T& element)
    {
        LockType lock{mutex};

        cv.wait(lock, [&]() { return enabled == false || !queue.empty(); });

        if (!queue.empty()) {
            element = std::move(queue.front());
            queue.pop();
            return true;
        }

        return false;
    }
};
}

#endif
