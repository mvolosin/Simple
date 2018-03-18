#include <iostream>
#include <thread>

#include "Simple/ConditionBuffer.hpp"
#include "Simple/Logger.hpp"

struct MyObject {
    int x;
    int y;
};

std::ostream& operator<<(std::ostream& os, const MyObject& obj)
{
    os << "X: " << obj.x << " Y: " << obj.y;
    return os;
}

std::mutex mtx;
void log(MyObject& obj)
{
    std::lock_guard<std::mutex> lock(mtx);
    LOG_INFO << "TID: " << std::this_thread::get_id() << " " << obj;
}

void logEnd()
{
    std::lock_guard<std::mutex> lock(mtx);
    LOG_INFO << "TID: " << std::this_thread::get_id() << " ended";
}

void logFullBuffer()
{
    std::lock_guard<std::mutex> lock(mtx);
    LOG_INFO << "Buffer is full";
}

int main()
{
    Simple::ConditionBuffer<MyObject, 1000> buffer;

    std::thread producer([&]() {
        for (int i = 0; i < 1000; ++i) {
            if (!buffer.put({i, -i}))
                logFullBuffer();
        }
    });

    std::vector<std::thread> consumers;
    for (int i = 0; i < 10; ++i) {
        consumers.push_back(std::thread([&]() {
            MyObject object;
            while (buffer.get(object)) {
                log(object);
            }
            logEnd();
        }));
    }

    producer.join();
    buffer.disable();

    for (auto& c : consumers)
        c.join();
}
