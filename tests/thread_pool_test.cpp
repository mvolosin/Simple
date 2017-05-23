#include "../include/ThreadPool.hpp"
#define INIT_SIMPLE_LOGGER
#include "../include/Logger.hpp"
#include <iostream>
#include <thread>

using namespace std;

void task()
{
    size_t s = 0;
    for (; s < 100000; ++s){
        ;
    }
    LOG(LogLevel::INFO) << "Result = " << s;
};

int main(int argc, char *argv[])
{
    Simple::ThreadPool<void()> pool(10);

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < 100; ++i) {
        futures.push_back(pool.addTask([]() { task(); }));
    }

    for(auto& f : futures){
        f.get();
    }

    return 0;
}
