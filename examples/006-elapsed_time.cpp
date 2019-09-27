#include <gtest/gtest.h>

#include "../src/exmm.hpp"

#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

using namespace ExMM;

struct Registers
{
    volatile int Time;
    volatile int Other[10];
};

struct Controller006 final : public ControllerBase<HookTypes::Read, Registers>
{
    Controller006() : whenStarted(std::chrono::system_clock::now())
    {}
    
    void HookRead(Registers *data, size_t offset) override
    {
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
        
        SwitchField(data, offset)
                .Case(&Registers::Time, [this](auto &time) {
                    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now() - whenStarted);
                    time = static_cast<int>(ms.count());
                });
    }

private:
    std::chrono::system_clock::time_point whenStarted;
};

TEST(ElapsedTimeRegisterAccessCase, elapsedTimeRegisterAccess)
{
    std::cout << "Register Time reads time elapsed since controller start"
              << std::endl;
    
    Controller006 controller;
    auto *registers = controller.GetIoArea();
    
    std::vector<int> values;
    
    ExMM::Run([&values, &registers]() {
        std::this_thread::sleep_for(std::chrono::milliseconds{std::rand() % 2000});
        
        int x = registers->Time;
        values.push_back(x);
        std::cout << "[0] Current time: " << std::dec << x << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds{1});
        
        x = registers->Time;
        values.push_back(x);
        std::cout << "[1] Current time: " << std::dec << x << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds{2});
        
        x = registers->Time;
        values.push_back(x);
        std::cout << "[2] Current time: " << std::dec << x << std::endl;
    });
    
    const int delta = values[1] - values[0];
    const int delta2 = values[2] - values[0];
    
    EXPECT_EQ(values.size(), 3);
    EXPECT_GT(delta, 950);
    EXPECT_LT(delta, 1050);
    EXPECT_GT(delta2, 2900);
}
