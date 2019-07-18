#include "../utilities/examples-registry.hpp"
#include <iostream>

int main(int, char**)
{
    return 
    ExamplesRegistry::RunAll(
        std::cout,
        [](auto& callback) {std::cout << "[" << callback.Name << "]" << std::endl; },
        [](auto& callback) {std::cout << "<complete>" << std::endl << std::endl; },
        [](auto& callback) {std::cerr << "<failed>" << std::endl << std::endl;}
        ) ? 0 : 1;
}
