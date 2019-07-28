#include "../utilities/examples-registry.hpp"

#include "../src/exmm.hpp"

#include <iostream>
#include <set>
#include <cstddef>
#include <vector>

using namespace ExMM;

struct Registers
{
    volatile int A;
    volatile int B;
    volatile int C;
};

struct Controller004 final : public ControllerBase<HookTypes::Read, Registers>
{
    void HookRead(Registers* data, size_t offset) override
    {
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
        
        Field(data, offset)
        .Match<volatile int>(&Registers::A, [](volatile int& f)
        {
            ++f;
            std::cout << "Field A incremented" << std::endl;
        })
        .Else([]()
        {
            std::cout << "Non-observed field" << std::endl;
        });
    }
};

EXMM_DEMO(ReadAccessCounter)
{
    output << "Register A counts read access" << std::endl;

    Controller004 controller;
    auto *registers = controller.GetIoSpace();

    std::vector<int> values;

    ExMM::Run([&values, &registers]()
    {
        int x = registers->A;
        values.push_back(x);
        x = registers->A;
        values.push_back(x);
        x = registers->A;
        values.push_back(x);

        x = registers->C;
        values.push_back(x);
    });

    return values.size() == 4
        && values[0] == 1
        && values[1] == 2
        && values[2] == 3
        && values[3] == 0
        ;
}
