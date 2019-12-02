#include <benchmark/benchmark.h>
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


static void BM_ReadWrite(benchmark::State& state)
{
    volatile int x = 0;
    int result = 0;
    ExMM::Run([&x, &result, &state] {
           
            for(auto _ : state)
            {
                      result += x++;
            }
    });
    
    
}

BENCHMARK(BM_ReadWrite);



static void BM_ReadWriteRegister(benchmark::State& state)
{
    Controller900a controller;
    
    volatile Dummy* registers = controller.GetIoArea();
    int result = 0;
    

        ExMM::Run([registers, &result, &state]
                  {
                      for(auto _ : state)
                      {
                          result += registers->X;
                      }
                  });
    
}

BENCHMARK(BM_ReadWriteRegister);


static void BM_Counter(benchmark::State& state)
{
    Controller900b controller;
    
    volatile Dummy* registers = controller.GetIoArea();
    
    int result = 0;
 
        ExMM::Run([registers, &result, &state]
                  {
                 
                      for(auto _ : state)
                      {
                      result += registers->X;
                      }
                  });
   
}

BENCHMARK(BM_Counter);


static void BM_OneShotDsl(benchmark::State& state)
{
    Controller900c controller;
    
    volatile Dummy* registers = controller.GetIoArea();
    
    int result = 0;
    
    ExMM::Run([registers, &result, &state]
              {
              
                  for(auto _ : state)
                  {
                      result += registers->X;
                  }
              });
    
}

BENCHMARK(BM_OneShotDsl);


BENCHMARK_MAIN();