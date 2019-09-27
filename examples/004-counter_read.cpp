#include <gtest/gtest.h>

#include "../src/exmm.hpp"

#include <iostream>
#include <cstddef>
#include <vector>

using namespace ExMM;

struct Registers
{
    volatile int A;
    volatile int B;
    volatile int C;
};

struct Controller004 final : public ControllerBase<HookTypes::Read, Registers>
{
    void HookRead(Registers *data, size_t offset) override
    {
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
        
        SwitchField(data, offset)
                .Case(&Registers::A, [](auto &a) {
                    a += 1;
                    std::cout << "Field A incremented" << std::endl;
                })
                .Case(&Registers::B, [data](auto &b) {
                    b += data->C;
                    std::cout << "Field B increased by value of field C" << std::endl;
                })
                .Else([](std::size_t offset) {
                    std::cout << "Non-observed field at offset " << offset << std::endl;
                });
    }
};

TEST(ReadAccessCounterCase, readAccessCounter)
{
    std::cout << "Register A counts read access" << std::endl;
    
    Controller004 controller;
    auto *registers = controller.GetIoArea();
    
    std::vector<int> values;
    
    ExMM::Run([&values, &registers]() {
        int x = registers->A;
        values.push_back(x);
        
        x = registers->A;
        values.push_back(x);
        
        x = registers->A;
        values.push_back(x);
        
        x = registers->C;
        values.push_back(x);
        
        registers->C = 5;
        x = registers->B;
        values.push_back(x);
        
        registers->C = 12;
        x = registers->B;
        values.push_back(x);
        
        registers->C = -3;
        x = registers->B;
        values.push_back(x);
    });
    
    EXPECT_EQ(values.size(), 7);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 3);
    EXPECT_EQ(values[3], 0);
    EXPECT_EQ(values[4], 5);
    EXPECT_EQ(values[5], 17);
    EXPECT_EQ(values[6], 14);
    
}
