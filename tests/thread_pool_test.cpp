#include "../include/ThreadPool.hpp"
#define INIT_SIMPLE_LOGGER
#include "../include/Logger.hpp"
#include <iostream>
#include <thread>

using namespace std;

void task(size_t id)
{
    size_t s = 0;
    for (; s < 100000; ++s){
        ;
    }
    LOG(LogLevel::INFO) << "Task ID " << id << " result = " << s;
};

size_t task2(size_t id)
{
    size_t s = 0;
    for (; s < 100000; ++s){
        ;
    }
    LOG(LogLevel::INFO) << "Task ID " << id << " result = " << s;
    return s;
};

int main(int argc, char *argv[])
{
    Simple::ThreadPool<void()> pool(10);

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < 100; ++i) {
        futures.push_back(pool.addTask([=]() { task(i); }));
    }

    for(auto& f : futures){
        f.get();
    }

    Simple::ThreadPool<size_t()> pool2(10);

    std::vector<std::future<size_t>> futures2;
    for (size_t i = 0; i < 100; ++i) {
        futures2.push_back(pool2.addTask([i]() { return task2(i); }));
    }

    for(auto& f : futures2){
        auto s = f.get();
    }

    return 0;
}
