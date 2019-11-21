#include <gtest/gtest.h>

#include "../src/exmm.hpp"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

using namespace ExMM;

struct Registers
{
    volatile uint32_t TimeLo; // Microseconds
    volatile uint32_t TimeHi; // Integral seconds
    volatile int Other[10];
};

struct Controller007 final : public ControllerBase<HookTypes::Read, Registers>
{
    Controller007()
            : whenStarted(std::chrono::high_resolution_clock::now()),
              latchedLoValue()
    {}
    
    void HookRead(volatile Registers *data, size_t offset) override
    {
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
        
        SwitchField(data, offset)
                .Case(
                        &Registers::TimeLo,
                        [this](auto &timeLo) { timeLo = latchedLoValue; })
                .Case(&Registers::TimeHi, [this](auto &timeHi) {
                    static const std::chrono::seconds oneSecond{1};
                    
                    const auto elapsed =
                            std::chrono::high_resolution_clock::now() - whenStarted;
                    const auto us =
                            std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
                    const auto seconds = us / oneSecond;
                    
                    latchedLoValue = static_cast<uint32_t>((us % oneSecond).count());
                    timeHi = static_cast<uint32_t>(seconds);
                });
    }

private:
    std::chrono::high_resolution_clock::time_point whenStarted;
    uint32_t latchedLoValue;
};

TEST(LatchedTimeAccessCase, latchedTimeAccess)
{
    std::cout << "Access time latched in register pair when accessed highest part"
              << std::endl;

    const Controller007 controller;
    auto *registers = controller.GetIoArea();
    
    std::vector<uint32_t> values;
    
    ExMM::Run([&values, &registers]() {
        std::this_thread::sleep_for(std::chrono::milliseconds{std::rand() % 2000});
        
        uint32_t sec = registers->TimeHi;
        uint32_t usec = registers->TimeLo;
        
        values.push_back(sec);
        values.push_back(usec);
        
        std::cout << "[0] Current time: " << std::dec << sec << "."
                  << std::setfill('0') << std::setw(6) << usec << std::endl;
        
        std::this_thread::sleep_for(
                std::chrono::milliseconds{std::rand() % 2000 + 1000});
        
        usec = registers->TimeLo;
        values.push_back(usec);
        
        std::this_thread::sleep_for(
                std::chrono::milliseconds{std::rand() % 2000 + 1000});
        
        sec = registers->TimeHi;
        usec = registers->TimeLo;
        
        values.push_back(sec);
        values.push_back(usec);
        
        std::cout << "[1] Current time: " << std::dec << sec << "."
                  << std::setfill('0') << std::setw(6) << usec << std::endl;
    });
    
    EXPECT_EQ(values.size(), 5);
    EXPECT_EQ(values[1], values[2]);
    EXPECT_NE(values[1], values[4]);
}
