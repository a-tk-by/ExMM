#include "../utilities/examples-registry.hpp"

#include "../src/exmm.hpp"

#include <iostream>
#include <set>

using namespace ExMM;

volatile struct Registers
{
    volatile int A;
    volatile int B;
    volatile int C;
};

struct Controller002 final : public ControllerBase<HookTypes::ReadWrite, Registers>
{
    std::set<size_t> readOffsets;
    std::set<size_t> writeOffsets;

    void HookRead(Registers* data, size_t offset) override
    {
        readOffsets.insert(offset);
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
    }

    void HookWrite(Registers* data, size_t offset) override
    {
        writeOffsets.insert(offset);
        std::cout << "After write at offset " << std::hex << offset << std::endl;
    }
};

EXMM_DEMO(SimplePassiveHooks)
{
    output << "Basic demo - simple passive hooks" << std::endl;

    Controller002 controller;
    auto *registers = controller.GetIoSpace();

    registers->A = 42;
    registers->B = 123;
    registers->C = -1;

    int sum = registers->A + registers->B + registers->C;


    const auto notFoundRead = controller.readOffsets.end();
    const auto notFoundWrite = controller.writeOffsets.end();

    return sum == (42 + 123 - 1)
        && controller.readOffsets.find(0) != notFoundRead
        && controller.readOffsets.find(4) != notFoundRead
        && controller.readOffsets.find(8) != notFoundRead
        && controller.readOffsets.find(1) == notFoundRead
        && controller.writeOffsets.find(0) != notFoundWrite
        && controller.writeOffsets.find(4) != notFoundWrite
        && controller.writeOffsets.find(8) != notFoundWrite
        && controller.writeOffsets.find(1) == notFoundWrite
        ;
}
