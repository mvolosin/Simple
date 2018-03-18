#ifndef SIMPLE_SIGNAL_HPP
#define SIMPLE_SIGNAL_HPP

#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>

namespace Simple {
template <class T>
class Signal;

template <class T, class... Args>
class Signal<T(Args...)> {
    using Slot = std::function<T(Args...)>;
    using Lock = std::lock_guard<std::mutex>;
    std::vector<Slot> slots_;
    std::mutex mtx_;

public:
    Signal() = default;

    /**
     * @brief Connect slot to this signal
     *
     * @param s
     */
    bool connect(Slot s)
    {
        if (s == nullptr)
            return false;
        Lock l{mtx_};
        slots_.push_back(s);
        return true;
    }

    /**
     * @brief Same as connect()
     *
     * @param s
     *
     * @return
     */
    bool operator+=(Slot s)
    {
        return connect(s);
    }

    /**
     * @brief Call all connected slots with arguments
     *
     * @param args
     */
    void emit(Args... args)
    {
        Lock l{mtx_};
        for (auto& s : slots_) {
            s(args...);
        }
    }

    /**
     * @brief Call all connected slots with arguments
     *
     * @param args
     */
    void operator()(Args... args)
    {
        emit(std::forward<Args>(args)...);
    }

    /**
     * @brief Clean all slots
     */
    void disconnectAll()
    {
        Lock l{mtx_};
        slots_.clear();
    }
};
} // namespace Simple

#endif /* ifndef SIMPLE_SIGNAL_HPP */
