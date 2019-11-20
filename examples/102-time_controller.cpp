#include <gtest/gtest.h>

#include "../src/exmm.hpp"

#include <iostream>
#include <cstddef>
#include <vector>
#include <atomic>
#include <iomanip>
#include <thread>

using namespace ExMM;

struct Registers
{
    struct ControlBits_t
    {
        uint32_t Start
                : 1;             // Controller starts counting when 1 is written in this bit. Controller restarts every time when 1 is written here. This bit always reads as 0.
        
        uint32_t : 7;
        
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
        
        // Timer<X>IE - Interrupt enabled (1) for chosen timer
        uint32_t Timer0IE : 1;
        uint32_t Timer1IE : 1;
        uint32_t Timer2IE : 1;
        uint32_t Timer3IE : 1;
        uint32_t Timer4IE : 1;
        uint32_t Timer5IE : 1;
        uint32_t Timer6IE : 1;
        uint32_t Timer7IE : 1;
        
        enum : uint32_t
        {
            TimerActiveMask = 0xFFul << 16,
            TimerInterruptMask = 0xFFul << 24,
        };
        enum : uint32_t
        {
            TimerActiveShift = 16,
            TimerInterruptShift = 24,
        };
    };
    
    struct TimerData
    {
        uint32_t TriggerTimeHi;         // Time when timer triggers, seconds.
        uint32_t TriggerTimeLo;         // Time when timer triggers, microseconds.
    };
    
    struct StatusBits_t
    {
        uint32_t : 16;
        uint32_t : 8;
        
        uint32_t Timer0Interrupt : 1;
        uint32_t Timer1Interrupt : 1;
        uint32_t Timer2Interrupt : 1;
        uint32_t Timer3Interrupt : 1;
        uint32_t Timer4Interrupt : 1;
        uint32_t Timer5Interrupt : 1;
        uint32_t Timer6Interrupt : 1;
        uint32_t Timer7Interrupt : 1;
        
        enum : uint32_t
        {
            TimerInterruptMask = 0xFFul << 24,
        };
        enum : uint32_t
        {
            TimerInterruptShift = 24,
        };
    };
    
    
    union
    {
        uint32_t ControlWord;                  // Control register as whole word.
        ControlBits_t ControlBits;             // Control register as bit fields.
    };
    
    union
    {
        uint32_t StatusWord;                  // Status register as whole word.
        StatusBits_t StatusBitsBits;          // Status register as bit fields.
    };
    
    uint32_t InitialTime;                     // Time counter which value will be latched on timer start.
    
    uint32_t CurrentTimeHi;                   // Current time, seconds.
    uint32_t CurrentTimeLo;                   // Current time, microseconds. This register is latched when CurrentTimeHi is read.
    
    volatile uint32_t NotUsed[3];
    
    volatile TimerData Timers[8];                      // Timers' configuration.
    
};

static_assert(sizeof(Registers::ControlBits_t) == sizeof(uint32_t), "Control bits must be 32-bit word");
static_assert(sizeof(Registers::StatusBits_t) == sizeof(uint32_t), "Status bits must be 32-bit word");

struct Controller009 final : public ControllerBase<HookTypes::ReadWrite, Registers>
{
    Controller009()
            : latchedCurrentTimeLo(), shadowControl(), shadowStatus(), whenStartedRegisterTime(),
              timerThreadControl(TimerThreadControl::Idle),
              timerThread(std::thread([this]() {
                  this->TimerThread();
              }))
    {
    
    }
    
    ~Controller009()
    {
        timerThreadControl = TimerThreadControl::Abort;
        timerThread.join();
    }
    
    void HookRead(volatile Registers *data, size_t offset) override
    {
        std::cout << "Hook read #" << offset << " : ";
        
        SwitchField(data, offset)
                .Case(&Registers::ControlWord, [this](auto &value) {
                    value = shadowControl;
                    std::cout << value << " ControlWord";
                })
                .Case(&Registers::StatusWord, [this](auto &value) {
                    value = shadowStatus;
                    std::cout << value << " StatusWord";
                })
                .Case(&Registers::CurrentTimeHi, [this](auto &value) {
                    value = GetCurrentTime(latchedCurrentTimeLo);
                    std::cout << value << " CurrentTimeHi";
                })
                .Case(&Registers::CurrentTimeLo, [this](auto &value) {
                    value = latchedCurrentTimeLo;
                    std::cout << value << " CurrentTimeLo";
                })
                .CaseArray(&Registers::NotUsed, [](size_t, auto &value) {
                    value = 0;
                    std::cout << value << " NotUsed";
                })
                .Else([data](size_t offset) {
                    std::cout << "((not observed)) " << reinterpret_cast<volatile uint32_t *>(data)[offset / 4];
                });
        
        std::cout << std::endl;
    }
    
    void HookWrite(volatile Registers *data, size_t offset) override
    {
        std::cout << "Hook write #" << offset << " : ";
        
        SwitchField(data, offset)
                .Case(&Registers::ControlWord, [this](auto &value) {
                    std::cout << value;
                    value = this->ControlChanged(value);
                    std::cout << " -> " << value;
                })
                .Case(&Registers::StatusWord, [this](auto &value) {
                    std::cout << value;
                    shadowStatus &= ~(value & Registers::StatusBits_t::TimerInterruptMask); // Atomic reset
                })
                .InsideArray(&Registers::Timers,
                             [](std::size_t index, auto &next) {
                                 std::cout << " timer configuration data";
                                 next
                                         .Case(&Registers::TimerData::TriggerTimeHi,
                                               [](auto &value) {
                                                   std::cout << " hi " << value;
                                               })
                                         .Case(&Registers::TimerData::TriggerTimeLo,
                                               [](auto &value) {
                                                   std::cout << " lo " << value;
                                                   value = std::min(uint32_t{value},
                                                                    999999u);
                                                   std::cout << " -> " << value;
                                               });
                             })
                .Else([data](size_t offset) {
                    std::cout << "((not observed)) " << reinterpret_cast<volatile uint32_t *>(data)[offset / 4];
                });
        
        std::cout << std::endl;
    }

private:
    
    uint32_t latchedCurrentTimeLo;
    
    std::atomic<uint32_t> shadowControl;
    std::atomic<uint32_t> shadowStatus;
    
    std::thread timerThread;
    
    enum class TimerThreadControl
    {
        Idle,
        Abort,
        Running,
        Restart,
    };
    
    std::atomic<TimerThreadControl> timerThreadControl;
    
    uint32_t ControlChanged(uint32_t value)
    {
        std::cout << std::endl << "Control register changed from " << shadowControl << " to " << value
                  << " and then to ";
        
        Registers::ControlBits_t &bits = *reinterpret_cast<Registers::ControlBits_t *>(&value);
        
        if (bits.Start) {
            timerThreadControl = TimerThreadControl::Restart;
            bits.Start = false;
        }
        
        std::cout << value << std::endl;
        
        return shadowControl = value;
    }
    
    uint32_t GetCurrentTime(uint32_t &lowPart)
    {
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - whenStarted).count();
        lowPart = duration % 1000000;
        return static_cast<uint32_t>(whenStartedRegisterTime + duration / 1000000);
    }
    
    void RaiseAndResetTimer(int i, uint32_t timeHi, uint32_t timeLo, volatile const Registers::TimerData *timers)
    {
        if (shadowControl & (1 << (i + Registers::ControlBits_t::TimerActiveShift))) {
            bool triggerCurrentTimer = false;
            if (timeHi > timers[i].TriggerTimeHi) {
                triggerCurrentTimer = true;
            } else if (timeHi, timers[i].TriggerTimeHi && timeLo > timers[i].TriggerTimeLo) {
                triggerCurrentTimer = true;
            }
            
            std::cout << "Timer #" << i << " @ " << timers[i].TriggerTimeHi << " " << timers[i].TriggerTimeLo <<
                      " elapsed:" << (triggerCurrentTimer ? " yes " : " no ") << std::endl;
            
            if (triggerCurrentTimer) {
                shadowControl &= ~(1 << (i + Registers::ControlBits_t::TimerActiveShift));
                shadowStatus |= (1 << (i + Registers::StatusBits_t::TimerInterruptShift));
            }
        }
    }
    
    void TimerThread()
    {
        while (true)
            switch (timerThreadControl) {
                case TimerThreadControl::Abort:
                    std::cout << std::endl << "Time thread exit" << std::endl;
                    return;
                case TimerThreadControl::Idle:
                    std::this_thread::yield();
                    break;
                case TimerThreadControl::Restart:
                    std::cout << std::endl << "Time thread restart" << std::endl;
                    whenStarted = std::chrono::high_resolution_clock::now();
                    {
                        std::lock_guard<std::recursive_mutex> guard(AcquireMemoryLock());
                        whenStartedRegisterTime = this->GetPrivateIoArea()->InitialTime;
                    }
                    timerThreadControl = TimerThreadControl::Running;
                    std::cout << std::endl << "Time thread is now running" << std::endl;
                    break;
                default:
                    uint32_t timeLo, timeHi = GetCurrentTime(timeLo);
                    std::cout << "Time: " << timeHi << " " << timeLo << std::endl;
                    
                    {
                        std::lock_guard<std::recursive_mutex> guard(AcquireMemoryLock());
                        
                        const auto *timers = this->GetPrivateIoArea()->Timers;
                        
                        for (int i = 0; i < 8; ++i) {
                            RaiseAndResetTimer(i, timeHi, timeLo, timers);
                        }
                    }
                    
                    const auto status = ((shadowStatus.load() & Registers::StatusBits_t::TimerInterruptMask)
                            >> Registers::StatusBits_t::TimerInterruptShift);
                    const auto control = ((shadowControl.load() & Registers::ControlBits_t::TimerInterruptMask)
                            >> Registers::ControlBits_t::TimerInterruptShift);
                    
                    std::cout << std::hex << std::setw(8) << "[S:C] " << shadowStatus << " " << shadowControl
                              << std::endl;
                    std::cout << std::hex << std::setw(8) << "[s:c] " << status << " " << control << " & "
                              << (status & control) << std::endl;
                    
                    if (status & control) {
                        this->TriggerInterrupt();
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds{200});
                    break;
            }
    }
    
    std::chrono::high_resolution_clock::time_point whenStarted;
    uint32_t whenStartedRegisterTime;
};

TEST(TimeControllerCase, timeController)
{
    
    std::cout << "Simple time controller like in real world" << std::endl;
    
    Controller009 controller;
    auto *registers = controller.GetIoArea();
    
    std::vector<uint32_t> values;
    
    std::atomic_int interruptsTriggered{0};
    
    controller.ConnectInterruptHandler(0, [&interruptsTriggered, registers]() {
        ExMM::Run([&interruptsTriggered, registers]() {
            std::cout << "Interrupt triggered" << std::endl;
            const uint32_t flags = registers->StatusWord;
            ++interruptsTriggered;
            registers->StatusWord = flags;
        });
    });
    
    ExMM::Run([&values, &registers]() {
        std::cout << "Initializing timers" << std::endl;
        
        registers->InitialTime = 100;
        values.push_back(uint32_t{registers->InitialTime});
        
        registers->Timers[1].TriggerTimeHi = 105;
        registers->Timers[1].TriggerTimeLo = 250000;
        values.push_back(uint32_t{registers->Timers[1].TriggerTimeHi});
        values.push_back(uint32_t{registers->Timers[1].TriggerTimeLo});
        
        registers->ControlBits.StartTimer1 = 1;
        registers->ControlBits.Timer1IE = 1;
        
        registers->Timers[3].TriggerTimeHi = 110;
        registers->Timers[3].TriggerTimeLo = 2000000;
        values.push_back(uint32_t{registers->Timers[3].TriggerTimeHi});
        values.push_back(uint32_t{registers->Timers[3].TriggerTimeLo});
        
        registers->ControlBits.StartTimer3 = 1;
        registers->ControlBits.Timer3IE = 0;
        
        registers->ControlBits.Start = 1;
        
        std::cout << "Waiting for results" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds{15});
        
        std::cout << "Checking" << std::endl;
        
        values.push_back(registers->ControlWord & Registers::ControlBits_t::TimerActiveMask);
        values.push_back(registers->StatusWord & Registers::StatusBits_t::TimerInterruptMask);
    });
    
    EXPECT_EQ(values.size(), 7);
    EXPECT_EQ(values[0], 100);
    EXPECT_EQ(values[1], 105);
    EXPECT_EQ(values[2], 250000);
    EXPECT_EQ(values[3], 110);
    EXPECT_EQ(values[4], 999999);
    EXPECT_EQ(interruptsTriggered, 1);
    EXPECT_EQ(values[5], 0);
    EXPECT_NE(values[6], 0);
}

