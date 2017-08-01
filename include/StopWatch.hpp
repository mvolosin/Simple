#ifndef SIMPLE_STOPWATCH_HPP
#define SIMPLE_STOPWATCH_HPP

#include <chrono>
#include <iostream>
#include <string>

namespace Simple {
class StopWatch {
    using ClockType = std::chrono::high_resolution_clock;
    using TimePointType = ClockType::time_point;

    std::string name_;
    TimePointType begin_;
    bool printed_;

public:
    StopWatch(std::string name)
        : name_(name)
        , begin_(ClockType::now())
        , printed_(false)
    {
    }

    ~StopWatch()
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
