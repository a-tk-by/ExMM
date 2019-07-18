#include "../utilities/examples-registry.hpp"
#include <iostream>

#include "../src/exmm.hpp"

using namespace ExMM;

volatile struct Registers
{
    volatile int A;
    volatile int B;
    volatile int C;
};

struct Controller final : public ControllerBase<HookTypes::None, Registers>
{
    bool WaitForInterrupt()
    {
        bool success = false;

        ConnectInterruptHandler(0, [controller = this, &success]()
        {
            std::cout << "Interrupt triggered" << std::endl;
            success = true;
        });

        std::thread([controller = this]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            controller->TriggerInterrupt(0);
        }).join();

        return success;
    }

};

EXMM_DEMO(BasicInterrupt)
{
    output << "Basic demo - Interrupt" << std::endl;

    bool triggered;
    {
        Controller controller;
        triggered = controller.WaitForInterrupt();
    }

    return triggered;
}
