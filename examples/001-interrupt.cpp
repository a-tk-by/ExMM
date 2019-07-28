#include "../utilities/examples-registry.hpp"
#include <iostream>
#include <thread>
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

        std::thread([&]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            TriggerInterrupt(0);
        }).join();

        return success;
    }

};

EXMM_DEMO(BasicInterrupt)
{
    output << "Basic demo - Interrupt" << std::endl;

    bool triggered;
    {
        Controller001 controller;
        triggered = controller.WaitForInterrupt();
    }

    return triggered;
}
