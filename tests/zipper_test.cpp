#include "Simple/Zipper.hpp"

#include <iostream>
#include <list>
#include <thread>
#include <vector>

using namespace std;

int main(int argc, char* argv[])
{

    std::vector<int> ints{10, 20, 30, 40};
    std::list<char> chars{'a', 'b', 'c', 'd'};

    auto zipped = Simple::Zipper(ints, chars);

    for(auto& z : zipped) {
        std::cout << "int: " << std::get<0>(z)
            << " char: " << std::get<1>(z) << std::endl;
    }

    return 0;
}
