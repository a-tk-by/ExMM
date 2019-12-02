#include <gtest/gtest.h>
#include "../src/exmm.hpp"

#include <chrono>
#define BENCHMARK_ITERATIONS 100000

struct Dummy
{
    volatile int X;
};

struct Controller900a final : public ExMM::ControllerBase<ExMM::HookTypes::ReadWrite, Dummy>
{

};

struct Controller900b final : public ExMM::ControllerBase<ExMM::HookTypes::ReadWrite, Dummy>
{
    int counter;

    void HookRead(volatile Dummy* data, size_t offset) override
    {
        data->X = counter++;
    }
};

struct Controller900c final : public ExMM::ControllerBase<ExMM::HookTypes::ReadWrite, Dummy>
{
    int counter;

    void HookRead(volatile Dummy* data, size_t offset) override
    {
        SwitchField(data, offset)
            .Case(&Dummy::X, [this](auto& v)
            {
                v = ++counter;
            })
            .Else([](std::size_t) {});
    }
};


TEST(Benchmark, Nothing)
{
    volatile int x = 0;

    ExMM::Run([&x]
    {
        int result = 0;

        const auto begin = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < BENCHMARK_ITERATIONS; ++i)
        {
            result += x++;
        }

        const auto end = std::chrono::high_resolution_clock::now();

        const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / BENCHMARK_ITERATIONS;
        std::cout << "Time per iteration is  " << std::dec << duration << " nanoseconds" << std::endl;
    });


    ASSERT_TRUE(true);
}


TEST(Benchmark, Empty)
{
    Controller900a controller;

    volatile Dummy* registers = controller.GetIoArea();

    ExMM::Run([registers]
    {
        int result = 0;

        const auto begin = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < BENCHMARK_ITERATIONS; ++i)
        {
            result += registers->X;
        }

        const auto end = std::chrono::high_resolution_clock::now();


        const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / BENCHMARK_ITERATIONS;
        std::cout << "Time per iteration is  " << std::dec << duration << " nanoseconds" << std::endl;
    });


    ASSERT_TRUE(true);
}


TEST(Benchmark, Counter)
{
    Controller900b controller;

    volatile Dummy* registers = controller.GetIoArea();

    ExMM::Run([registers]
    {
        int result = 0;

        const auto begin = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < BENCHMARK_ITERATIONS; ++i)
        {
            result += registers->X;
        }

        const auto end = std::chrono::high_resolution_clock::now();


        const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / BENCHMARK_ITERATIONS;
        std::cout << "Time per iteration is  " << std::dec << duration << " nanoseconds" << std::endl;
    });


    ASSERT_TRUE(true);
}


TEST(Benchmark, OneShotDsl)
{
    Controller900c controller;

    volatile Dummy* registers = controller.GetIoArea();

    ExMM::Run([registers]
    {
        int result = 0;

        const auto begin = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < BENCHMARK_ITERATIONS; ++i)
        {
            result += registers->X;
        }

        const auto end = std::chrono::high_resolution_clock::now();

        const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / BENCHMARK_ITERATIONS;
        std::cout << "Time per iteration is  " << std::dec << duration << " nanoseconds" << std::endl;    });


    ASSERT_TRUE(true);
}

