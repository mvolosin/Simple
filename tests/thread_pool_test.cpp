#include "../include/Logger.hpp"
#include "../include/ThreadPool.hpp"

#include <iostream>
#include <thread>

using namespace std;

int main(int argc, char *argv[])
{
    Simple::ThreadPool pool(10);

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < 100; ++i) {
        futures.push_back(pool.addTask(
            [](size_t id) {
                size_t s = 0;
                for (; s < 100000; ++s) {
                    ;
                }
                LOG_INFO << "Task ID " << id << " result = " << s;

            },
            i));
    }

    for (auto &f : futures) {
        f.get();
    }

    Simple::ThreadPool pool2(10);

    std::vector<std::future<size_t>> futures2;
    for (size_t i = 0; i < 100; ++i) {
        futures2.push_back(pool2.addTask(
            [](size_t id) {

                size_t s = 0;
                for (; s < 100000; ++s) {
                    ;
                }
                LOG_INFO << "Task ID " << id << " result = " << s;
                return s;
            },
            i));
    }

    for (auto &f : futures2) {
        auto s = f.get();
    }

    return 0;
}
