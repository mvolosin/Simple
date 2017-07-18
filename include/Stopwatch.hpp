#ifndef SIMPLE_STOPWATCH_HPP
#define SIMPLE_STOPWATCH_HPP

#include <chrono>
#include <iostream>
#include <string>

namespace Simple {
class Stopwatch {
    using TimePointType = std::chrono::time_point<std::chrono::system_clock>;
    using ClockType = std::chrono::high_resolution_clock;

    TimePointType begin_;
    std::string name_;
    bool printed_;

public:
    Stopwatch(std::string name)
        : name_(name)
        , begin_(ClockType::now())
        , printed_(false)
    {
    }

    ~Stopwatch()
    {
        if (!printed_) {
            print();
        }
    }

    void print()
    {
        auto diff = ClockType::now() - begin_;
        std::cout << "Duration of '" << name_
                  << "' = " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " ms"
                  << std::endl;
        printed_ = true;
    }
};
}

#endif /* ifndef SIMPLE_STOPWATCH_HPP */
