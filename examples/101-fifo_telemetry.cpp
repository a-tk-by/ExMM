#include <gtest/gtest.h>

#include "../src/exmm.hpp"

#include <iostream>
#include <cstddef>
#include <vector>
#include <queue>
#include <atomic>
#include <iomanip>
#include <thread>

using namespace ExMM;

struct Guard
{
    Guard(std::ostream& stream, std::recursive_mutex& otherMutex)
        : lock(otherMutex, mutex), stream(stream)
    {
    }

    std::ostream& operator*() const
    {
        return stream;
    }
private:
    static inline std::mutex mutex;

    std::scoped_lock<std::recursive_mutex, std::mutex> lock;
    std::ostream& stream;
};




struct Registers
{
    struct StatusFields_t
    {
        uint32_t StatusUsage : 8;
        uint32_t : 16;

        uint32_t StatusFifoEmpty : 1;
        uint32_t StatusFifoFull : 1;
        uint32_t StatusFifoHalfFull : 1;

        uint32_t : 4;

        uint32_t StatusInterruptFlag : 1;
    };

    volatile union
    {
        uint32_t        Status;        // Status register as whole word. Read-only.
        StatusFields_t  StatusFields;  // Status register as bit fields. Read-only.
        bool            NextRecord;    // Write-only. When written 1, fills telemetry data within next record.
    };

    volatile uint32_t Telemetry[4];
};

struct Controller008 final : public ControllerBase<HookTypes::ReadWrite, Registers>
{

    Controller008() : stopTmGenerator(false)
    {
        tmGenerator = std::thread([this]() {GenerateTelemetry(); });
    }

    ~Controller008()
    {
        stopTmGenerator = true;
        tmGenerator.join();
    }

    void GenerateTelemetry()
    {
        unsigned counter = 0;
        while (stopTmGenerator == false)
        {
            {
                std::lock_guard<std::recursive_mutex> guard(this->AcquireMemoryLock());
                if (fifo.size() < FifoMaxSize)
                {
                    TelemetryRecord buffer {{(1u << 24) + counter, (2 << 24) + counter, (4 << 24) + counter, (8 << 24) + counter}};

                    *Guard(std::cout, AcquireMemoryLock()) << "TM generated #" << std::dec << counter << std::endl;

                    fifo.push(buffer);

                    const auto newSize = fifo.size();

                    shadowStatus.Fields.StatusUsage = newSize;
                    if (newSize > FifoMaxSize / 2)
                    {
                        shadowStatus.Fields.StatusFifoHalfFull = true;
                    }
                    else if (newSize == FifoMaxSize/ 2)
                    {
                        shadowStatus.Fields.StatusFifoHalfFull = true;
                        this->TriggerInterrupt();
                    }
                    else if (newSize == FifoMaxSize)
                    {
                        shadowStatus.Fields.StatusFifoFull = true;
                        this->TriggerInterrupt();
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 20 + 10));
            ++counter;
        }
    }


    void HookRead(volatile Registers* data, size_t offset) override
    {
        SwitchField(data, offset)
        .Case(&Registers::Status, [this](auto& status)
        {
            status = shadowStatus.Word;
        })
        .CaseArray(&Registers::Telemetry, [this](std::size_t index, auto& tm)
        {
            tm = shadowRecord.Words[index];
        });
    }

    void HookWrite(volatile Registers* data, size_t offset) override
    {
        SwitchField(data, offset)
        .Case(&Registers::NextRecord, [this](auto& value)
        {
            if (value)
            {
                std::lock_guard<std::recursive_mutex> guard(AcquireMemoryLock());
                if (!fifo.empty())
                {
                    shadowRecord = fifo.front();
                    fifo.pop();

                    shadowStatus.Fields.StatusUsage = fifo.size();
                    shadowStatus.Fields.StatusFifoEmpty = fifo.empty();
                    shadowStatus.Fields.StatusFifoFull = fifo.size() < FifoMaxSize;
                    shadowStatus.Fields.StatusFifoHalfFull = fifo.size() >= FifoMaxSize / 2;
                }
            }
        });
    }

    std::recursive_mutex& GetLock()
    {
        return AcquireMemoryLock();
    }

private:

    struct TelemetryRecord
    {
        uint32_t Words[4];
    };

    std::atomic_bool stopTmGenerator;
    std::thread tmGenerator;

    std::queue<TelemetryRecord> fifo;
    static constexpr unsigned FifoMaxSize = 255;

    TelemetryRecord shadowRecord = {};
    union
    {
        Registers::StatusFields_t Fields;
        uint32_t Word;
    } shadowStatus = {};
};

int CheckCountersAndBits(std::vector<uint32_t>& values, std::size_t offset, std::size_t count);

TEST(FifoTelemetryReaderCase, fifoTelemetryReader)
{
    std::cout << "Registers access to FIFO buffer inside controller" << std::endl;

    Controller008 controller;
    auto *registers = controller.GetIoArea();

    std::atomic_bool interruptHandled;
    std::atomic<uint32_t> interruptStatus;

    controller.ConnectInterruptHandler(0, [registers, &interruptStatus, &interruptHandled, &controller]() // Memory access is guarded with mutex. Access to registers is thread-safe.
    {
        ExMM::Run([registers, &interruptStatus, &interruptHandled, &controller]()
        {
            interruptStatus = registers->Status;
            interruptHandled = true;
            *Guard(std::cout, controller.GetLock()) << "Controller interrupt: 0x" << std::hex << std::setw(8) << std::setfill('0') << interruptStatus << std::endl;
        });
    });

    std::vector<uint32_t> values;

    ExMM::Run([&values, &registers, &interruptHandled, &controller]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});

        const uint32_t status = registers->Status;
        values.push_back(status);
        *Guard(std::cout, controller.GetLock()) << "Status register: 0x" << std::hex << std::setw(8) << std::setfill('0') << status << std::endl;

        if (interruptHandled.exchange(false) == false)
        {
            *Guard(std::cout, controller.GetLock()) << "Failed to handle interrupt (timeout)" << std::endl;
            return;
        }

        // Before first record is latched
        values.push_back(uint32_t{ registers->Telemetry[0] });
        values.push_back(uint32_t{ registers->Telemetry[1] });
        values.push_back(uint32_t{ registers->Telemetry[2] });
        values.push_back(uint32_t{ registers->Telemetry[3] });

        // Latch record and read TM
        for (int i = 0; i < 1000; ++i)
        {
            if (registers->StatusFields.StatusFifoEmpty)
            {
                *Guard(std::cout, controller.GetLock()) << "FIFO is empty. Waiting for 0.5s" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds{500});
            }

            registers->NextRecord = true;
            values.push_back(uint32_t{ registers->Telemetry[0]});
            values.push_back(uint32_t{ registers->Telemetry[1]});
            values.push_back(uint32_t{ registers->Telemetry[2]});
            values.push_back(uint32_t{ registers->Telemetry[3]});
            *Guard(std::cout, controller.GetLock()) << "Reading next TM value... " << std::hex << std::setfill('0') << std::setw(8)
                << uint32_t{ registers->Telemetry[0] } << " "
                << uint32_t{ registers->Telemetry[1] } << " "
                << uint32_t{ registers->Telemetry[2] } << " "
                << uint32_t{ registers->Telemetry[3] }
                << std::endl;
        }
    });

    EXPECT_EQ(values.size(), (1 + 4 + 1000*4));
    EXPECT_GT(values[0] & 0xFFu, 0u);
    EXPECT_EQ(values[1], 0);
    EXPECT_EQ (values[2], 0);
    EXPECT_EQ(values[3], 0);
    EXPECT_EQ(values[4], 0);
    EXPECT_EQ(-1, CheckCountersAndBits(values, 5, 1000));
}

int CheckCountersAndBits(std::vector<uint32_t>& values, std::size_t offset, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i)
    {
        uint32_t tmp[4] = { values[offset + 4 * i], values[offset + 4 * i + 1], values[offset + 4 * i + 2], values[offset + 4 * i + 3]};
        if ((tmp[0] >> 24) != 1) return i;
        if ((tmp[1] >> 24) != 2) return i;
        if ((tmp[2] >> 24) != 4) return i;
        if ((tmp[3] >> 24) != 8) return i;

        if ((tmp[0] & 0xFFFF) != i) return i;
        if ((tmp[1] & 0xFFFF) != i) return i;
        if ((tmp[2] & 0xFFFF) != i) return i;
        if ((tmp[3] & 0xFFFF) != i) return i;
    }

    return -1;
}
