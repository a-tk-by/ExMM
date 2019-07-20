#include "../utilities/examples-registry.hpp"
#include <iostream>

int main(int, char**)
{
    return
    ExamplesRegistry::RunAll(
        std::cout,
        [](const ExamplesRegistry::Callback& callback) {std::cout << "[" << callback.Name << "]" << std::endl; },
        [](const ExamplesRegistry::Callback& callback) {std::cout << "<complete>" << std::endl << std::endl; },
        [](const ExamplesRegistry::Callback& callback) {std::cerr << "<failed>" << std::endl << std::endl;}
        ) ? 0 : 1;
}
