#include <gtest/gtest.h>
#include <iostream>
#include <future>
#include "../src/exmm.hpp"

using namespace ExMM;

struct Registers
{
    volatile int A;
    volatile int B;
    volatile int C;
};

struct Controller001 final : public ControllerBase<HookTypes::None, Registers>
{
    bool WaitForInterrupt()
    {
        bool success = false;

        ConnectInterruptHandler(0, [&success]()
        {
            std::cout << "Interrupt triggered" << std::endl;
            success = true;
        });

        std::async([&]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            TriggerInterrupt(0);
        });

        return success;
    }

};

TEST(BasicInterruptCase, basicInterrupt)
{
    std::cout << "Basic demo - Interrupt" << std::endl;

    bool triggered;
    {
        Controller001 controller;
        triggered = controller.WaitForInterrupt();
    }

    EXPECT_TRUE(triggered);
}
