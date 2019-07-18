#include "../utilities/examples-registry.hpp"
#include <iostream>

int main(int, char**)
{
    return 
    ExamplesRegistry::RunAll(
        std::cout,
        [](auto& callback) {std::cout << "[" << callback << "] success" << std::endl; },
        [](auto& callback) {std::cerr << "[" << callback << "] failed" << std::endl;}
        ) ? 0 : 1;
}
