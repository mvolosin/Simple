#include "../include/Signal.hpp"
#include <iostream>
#include <thread>

using namespace std;

class Object {
public:
    int id = 0;
    static int counter;

    Object(void)
    {
        id = ++counter;
        std::cout << "Object " << id << " constructed." << std::endl;
    }

    ~Object(void)
    {
        std::cout << "Object " << id << " destructed." << std::endl;
    }

public:
    void fun()
    {
        std::cout << "fun() member id = " << id << std::endl;
    }
};

int Object::counter = 0;

int main(int argc, char *argv[])
{
    Simple::Signal<void()> signal;
    {
        for (auto i = 0; i < 50; ++i) {
            Object ob1;
            signal.connect(std::bind(&Object::fun, &ob1));
            signal();
            signal.disconnectAll();
        }
    }
    signal.disconnectAll();

    signal.connect([]() { std::cout << "f1() called" << std::endl; });
    signal();

    return 0;
}
