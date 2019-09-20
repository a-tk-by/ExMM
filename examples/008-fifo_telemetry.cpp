#include "../utilities/examples-registry.hpp"

#include "../src/exmm.hpp"

#include <iostream>
#include <set>
#include <cstddef>
#include <vector>

using namespace ExMM;

struct Registers
{
    volatile int Time;
    volatile int Other[10];
};

struct Controller008 final : public ControllerBase<HookTypes::Read, Registers>
{
    void HookRead(Registers* data, size_t offset) override
    {
    }
private:
    std::chrono::system_clock::time_point whenStarted;
};

EXMM_DEMO(FifoTelemetryReader)
{
    output << "Registers access to FIFO buffer inside controller" << std::endl;

    Controller008 controller;
    auto *registers = controller.GetIoArea();

    std::vector<int> values;

    ExMM::Run([&values, &registers]()
    {
    });

    return values.size() < 0
        ;
}
