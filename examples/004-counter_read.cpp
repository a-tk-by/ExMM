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
        
        SwitchField(data, offset)
        .Case<int>(&Registers::A, [](volatile int& a)
        {
            a += 1;
            std::cout << "Field A incremented" << std::endl;
        })
        .Case<int>(&Registers::B, [](volatile Registers* all, volatile int& b)
        {
            b += all->C;
            std::cout << "Field B increased by value of field C" << std::endl;
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

        registers->C = 5;
        x = registers->B;
        values.push_back(x);

        registers->C = 12;
        x = registers->B;
        values.push_back(x);

        registers->C = -3;
        x = registers->B;
        values.push_back(x);
    });

    return values.size() == 7
        && values[0] == 1
        && values[1] == 2
        && values[2] == 3
        && values[3] == 0
        && values[4] == 5
        && values[5] == 17
        && values[6] == 14
        ;
}
