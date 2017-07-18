#ifndef SIMPLE_TIMER_HPP
#define SIMPLE_TIMER_HPP

#include <chrono>
#include <iostream>
#include <string>

namespace Simple {
class Timer {
    using TimePointType = std::chrono::time_point<std::chrono::system_clock>;

public:
    Timer(std::string name)
    {
        this->name = name;
        begin = std::chrono::system_clock::now();
    }

    void start()
    {
        end = std::chrono::system_clock::now();
    }

    void stop()
    {
        end = std::chrono::system_clock::now();
    }

    void stopAndPrint()
    {
        stop();
        print();
    }

    void print()
    {
        auto diff = end - begin;
        std::cout << "Duration of '" << name
                  << "' = " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " ms"
                  << std::endl;
    }

private:
    TimePointType begin;
    TimePointType end;
    std::string name;
};
}

#endif /* ifndef SIMPLE_TIMER_HPP */
