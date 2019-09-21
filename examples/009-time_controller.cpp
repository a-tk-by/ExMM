#include "../utilities/examples-registry.hpp"

#include "../src/exmm.hpp"

#include <iostream>
#include <cstddef>
#include <vector>
#include <atomic>
#include <iomanip>
#include <algorithm>
#include <thread>

using namespace ExMM;

struct Registers
{
    struct ControlBits_t
    {
        uint32_t Start : 1;             // Controller starts counting when 1 is written in this bit. Controller restarts every time when 1 is written here.
        uint32_t InterruptsEnabled : 1;

        uint32_t : 6;

        uint32_t : 8;

        // Timer<X> is active (1) or not (0)
        uint32_t StartTimer0 : 1;
        uint32_t StartTimer1 : 1; 
        uint32_t StartTimer2 : 1; 
        uint32_t StartTimer3 : 1; 
        uint32_t StartTimer4 : 1; 
        uint32_t StartTimer5 : 1; 
        uint32_t StartTimer6 : 1; 
        uint32_t StartTimer7 : 1;

        // Timer<X> is periodic (1) or not (0). If Timer<X>Period is 0, timer will not be periodic even if this flag is 1.
        uint32_t PeriodicTimer0 : 1;
        uint32_t PeriodicTimer1 : 1;
        uint32_t PeriodicTimer2 : 1;
        uint32_t PeriodicTimer3 : 1;
        uint32_t PeriodicTimer4 : 1;
        uint32_t PeriodicTimer5 : 1;
        uint32_t PeriodicTimer6 : 1;
        uint32_t PeriodicTimer7 : 1;

        enum : uint32_t
        {
            TimerActiveMask = 0xFFul << 16,
            TimerPeriodicMask = 0xFFul << 24,
        };
    };

    struct TimerData
    {
        uint32_t TriggerTimeHi;         // Time when timer triggers, seconds.
        uint32_t TriggerTimeLo;         // Time when timer triggers, microseconds.
        uint32_t Period;                // Period, microseconds.
    };

    struct StatusBits_t
    {
        uint32_t : 16;

        uint32_t Timer0Interrupt : 1;
        uint32_t Timer1Interrupt : 1;
        uint32_t Timer2Interrupt : 1;
        uint32_t Timer3Interrupt : 1;
        uint32_t Timer4Interrupt : 1;
        uint32_t Timer5Interrupt : 1;
        uint32_t Timer6Interrupt : 1;
        uint32_t Timer7Interrupt : 1;

        uint32_t : 0;
    };

    union
    {
        uint32_t Control;               // Control register as whole word.
        ControlBits_t ControlBits;      // Control register as bit fields.
    };

    union
    {
        uint32_t Status;                // Status register as whole word.
        StatusBits_t StatusBits;        // Status register as bit fields.
    };

    uint32_t InitialTime;               // Time counter which value will be latched on timer start.

    uint32_t CurrentTimeHi;             // Current time, seconds.
    uint32_t CurrentTimeLo;             // Current time, microseconds. This register is latched when CurrentTimeHi is read.

    TimerData Timers[8];                // Timers' configuration.
};

static_assert(sizeof(Registers::ControlBits_t) == sizeof(uint32_t), "Control bits must be 32-bit word");
static_assert(sizeof(Registers::StatusBits_t) == sizeof(uint32_t), "Status bits must be 32-bit word");

struct Controller009 final : public ControllerBase<HookTypes::ReadWrite, Registers>
{
    
    Controller009()
    {
    }

    ~Controller009()
    {
    }

    void HookRead(Registers* data, size_t offset) override
    {
        SwitchField(data, offset)
        .Case<uint32_t>(&Registers::Control, [this](volatile uint32_t& value)
        {
            value = shadowControl;
        })
        .Case<uint32_t>(&Registers::Status, [this](volatile uint32_t& value)
        {
            value = shadowStatus;
        })
        .Case<uint32_t>(&Registers::CurrentTimeHi, [this](volatile uint32_t& value)
        {
            value = GetCurrentTime(latchedCurrentTimeLo);
        });
    }

    void HookWrite(Registers* data, size_t offset) override
    {
        SwitchField(data, offset)
        .Case<uint32_t>(&Registers::Control, [this](volatile uint32_t& value)
        {
            this->ControlChanged(value);
        })
        .InsideArray<Registers::TimerData, 8>(&Registers::Timers, [](std::size_t index, FieldHelper<Registers::TimerData>& next)
        {
            next
            .Case<uint32_t>(&Registers::TimerData::TriggerTimeLo, [](volatile uint32_t& value)
            {
                 value = std::min(uint32_t{value}, 999999u);
            });
        });
    }

private:

    uint32_t latchedCurrentTimeLo;

    uint32_t shadowControl;
    uint32_t shadowStatus;

    uint32_t ControlChanged(uint32_t value)
    {
        return value;
    }

    uint32_t GetCurrentTime(uint32_t& lowPart)
    {
        lowPart = 0;
        return 0;
    }

};

EXMM_DEMO(TimeController)
{
    output << "Simple time controller like in real world" << std::endl;

    Controller009 controller;
    auto *registers = controller.GetIoArea();

    std::vector<uint32_t> values;

    ExMM::Run([&values, &registers]()
    {
    });

    return values.size() < 0
        ;
}

