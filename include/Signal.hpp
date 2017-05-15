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
        void connect(Slot s)
        {
            Lock l(mtx_);
            slots_.push_back(s);
        }

        /**
         * @brief Call all connected slots with arguments
         *
         * @param args
         */
        void emit(Args... args)
        {
            Lock l(mtx_);
            bool cleanAfter = false;
            for (auto &s : slots_) {
                if (s == nullptr) {
                    cleanAfter = true;
                    continue;
                }
                s(args...);
            }

            if (cleanAfter) {
                clean();
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
            Lock l(mtx_);
            slots_.clear();
        }

    private:
        inline void clean()
        {
            auto newEnd =
                std::remove_if(slots_.begin(), slots_.end(), [](const auto &slot) { return slot == nullptr; });
            slots_.erase(newEnd, slots_.end());
        }
    };
} // namespace Simple

#endif /* ifndef SIMPLE_SIGNAL_HPP */
