#include <gtest/gtest.h>

#include "../src/exmm.hpp"

#include <iostream>
#include <set>
#include <cstddef>

using namespace ExMM;

struct Registers
{
    volatile int A;
    volatile int B;
    volatile int C;
};

struct Controller003Write final : public ControllerBase<HookTypes::Write, Registers>
{
    std::set<size_t> readOffsets;
    std::set<size_t> writeOffsets;
    
    void HookRead(Registers *data, size_t offset) override
    {
        readOffsets.insert(offset);
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
    }
    
    void HookWrite(Registers *data, size_t offset) override
    {
        writeOffsets.insert(offset);
        std::cout << "After write at offset " << std::hex << offset << std::endl;
    }
};

struct Controller003Read final : public ControllerBase<HookTypes::Read, Registers>
{
    std::set<size_t> readOffsets;
    std::set<size_t> writeOffsets;
    
    void HookRead(Registers *data, size_t offset) override
    {
        readOffsets.insert(offset);
        std::cout << "Before read at offset " << std::hex << offset << std::endl;
    }
    
    void HookWrite(Registers *data, size_t offset) override
    {
        writeOffsets.insert(offset);
        std::cout << "After write at offset " << std::hex << offset << std::endl;
    }
};

TEST(HookCase, writeOnlyHook)
{
    std::cout << "Basic demo - only write hook" << std::endl;
    
    Controller003Write controller;
    auto *registers = controller.GetIoArea();
    
    int sum;
    ExMM::Run([&registers, &sum]() {
        registers->A = 42;
        registers->B = 123;
        registers->C = -1;
        
        sum = registers->A + registers->B + registers->C;
    });
    
    
    const auto notFoundRead = controller.readOffsets.end();
    const auto notFoundWrite = controller.writeOffsets.end();
    
    EXPECT_EQ(sum, (42 + 123 - 1));
    EXPECT_EQ(controller.readOffsets.find(0), notFoundRead);
    EXPECT_EQ(controller.readOffsets.find(4), notFoundRead);
    EXPECT_EQ(controller.readOffsets.find(8), notFoundRead);
    EXPECT_EQ(controller.readOffsets.find(1), notFoundRead);
    EXPECT_NE(controller.writeOffsets.find(0), notFoundWrite);
    EXPECT_NE(controller.writeOffsets.find(4), notFoundWrite);
    EXPECT_NE(controller.writeOffsets.find(8), notFoundWrite);
    EXPECT_EQ(controller.writeOffsets.find(1), notFoundWrite);
    
}

TEST(HookCase, readOnlyHook)
{
    std::cout << "Basic demo - only read hook" << std::endl;
    
    Controller003Read controller;
    auto *registers = controller.GetIoArea();
    
    int sum;
    
    ExMM::Run([&sum, &registers]() {
        registers->A = 42;
        registers->B = 123;
        registers->C = -1;
        
        sum = registers->A + registers->B + registers->C;
    });
    
    const auto notFoundRead = controller.readOffsets.end();
    const auto notFoundWrite = controller.writeOffsets.end();
    
    EXPECT_EQ(sum, (42 + 123 - 1));
    EXPECT_NE(controller.readOffsets.find(0), notFoundRead);
    EXPECT_NE(controller.readOffsets.find(4), notFoundRead);
    EXPECT_NE(controller.readOffsets.find(8), notFoundRead);
    EXPECT_EQ(controller.readOffsets.find(1), notFoundRead);
    EXPECT_EQ(controller.writeOffsets.find(0), notFoundWrite);
    EXPECT_EQ(controller.writeOffsets.find(4), notFoundWrite);
    EXPECT_EQ(controller.writeOffsets.find(8), notFoundWrite);
    EXPECT_EQ(controller.writeOffsets.find(1), notFoundWrite);
    
}
