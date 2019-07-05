#include "../utilities/examples-registry.hpp"
#include <iostream>

int main(int, char**)
{
	const ExamplesRegistry::Callback failed = ExamplesRegistry::RunAll(std::cout);
    if (failed)
    {
        std::cerr << "Failed callback: " << failed << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Demo complete" << std::endl;
        return 0;
    }
}
