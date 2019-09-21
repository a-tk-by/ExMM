#include "../utilities/examples-registry.hpp"

#include "../src/exmm.hpp"

#include <iostream>
#include <cstddef>
#include <vector>
#include <chrono>
#include <iomanip>
#include <thread>

using namespace ExMM;

struct Registers
{
    volatile uint32_t TimeLo; // Microseconds
    volatile uint32_t TimeHi; // Integral seconds
    volatile int Other[10];
};

struct Controller007 final : public ControllerBase<HookTypes::Read, Registers>
{
    Controller007() : whenStarted(std::chrono::high_resolution_clock::now()), latchedLoValue()
    {
    }

    void HookRead(Registers* data, size_t offset) override
    {
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
        
        SwitchField(data, offset)
        .Case<uint32_t>(&Registers::TimeLo, [this](volatile uint32_t& timeLo)
        {
            timeLo = latchedLoValue;
        })
        .Case<uint32_t>(&Registers::TimeHi,[this](volatile uint32_t& timeHi)
        {
            static const std::chrono::seconds oneSecond{1};

            const auto elapsed = std::chrono::high_resolution_clock::now() - whenStarted;
            const auto us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
            const auto seconds = us / oneSecond;

            latchedLoValue = static_cast<uint32_t>((us % oneSecond).count());
            timeHi = static_cast<uint32_t>(seconds);
        });
    }
private:
    std::chrono::high_resolution_clock::time_point whenStarted;
    uint32_t latchedLoValue;
};

EXMM_DEMO(LatchedTimeAccess)
{
    output << "Access time latched in register pair when accessed highest part" << std::endl;

    Controller007 controller;
    auto *registers = controller.GetIoArea();

    std::vector<uint32_t> values;

    ExMM::Run([&values, &registers]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ std::rand() % 2000});

        uint32_t sec = registers->TimeHi;
        uint32_t usec = registers->TimeLo;

        values.push_back(sec);
        values.push_back(usec);

        std::cout << "[0] Current time: " << std::dec << sec << "." << std::setfill('0') << std::setw(6) << usec << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds{ std::rand() % 2000 + 1000});

        usec = registers->TimeLo;
        values.push_back(usec);

        std::this_thread::sleep_for(std::chrono::milliseconds{ std::rand() % 2000 + 1000 });

        sec = registers->TimeHi;
        usec = registers->TimeLo;

        values.push_back(sec);
        values.push_back(usec);

        std::cout << "[1] Current time: " << std::dec << sec << "." << std::setfill('0') << std::setw(6) << usec << std::endl;
    });

    return values.size() == 5
        && (values[1] == values[2])
        && (values[1] != values[4])
        ;
}
