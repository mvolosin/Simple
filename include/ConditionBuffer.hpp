#ifndef SIMPLE_CONDITION_BUFFER_HPP
#define SIMPLE_CONDITION_BUFFER_HPP

#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace Simple {
    template<typename T, size_t size>
    class ConditionBuffer {
        using MutexType = std::mutex;
        using LockType = std::unique_lock<MutexType>;

        std::atomic_bool enabled;

        MutexType mutex;
        std::condition_variable cv;
        std::queue<T> queue;
        size_t maxBufferSize;

    public:
        ConditionBuffer(void)
            : maxBufferSize{ size }
            , enabled{ true }
        {};

        void disable()
        {
            enabled = false;
        }

        void enable()
        {
            enabled = true;
        }

        bool put(T&& element) {
            {
                LockType lock{ mutex };
                if (queue.size() == maxBufferSize)
                    return false;
                queue.push(std::forward<T>(element));
                cv.notify_all();
            }
            return true;
        }

        bool get(T& element) {
            LockType lock{ mutex };

            cv.wait(lock, [&]() {
                return enabled == false || !queue.empty();
            });

            if (!queue.empty()) {
                element = std::move(queue.front());
                queue.pop();
                return true;
            }
            else if (enabled)
                return false;
        }
    };
}

#endif
