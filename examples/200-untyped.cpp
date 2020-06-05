#include <gtest/gtest.h>

#include "../src/exmm.hpp"

using namespace ExMM;

struct Controller200 final : public ControllerBase<HookTypes::None>
{
    static constexpr size_t count = 1024;

    Controller200() : ControllerBase(1024 * sizeof(uint32_t))
    {
        std::cout << __FUNCTION__;
        std::getchar();

        volatile uint32_t* data = static_cast<volatile uint32_t*>(this->GetPrivateIoArea());
        for (size_t i = 0; i < count; ++i)
        {
            data[i] = 0xDEADBEEFu ^ i;
        }
    }
};

TEST(UntypedRegistersTestCase, untyped)
{
    std::cout << "Access to untyped registers file"
        << std::endl;

    const Controller200 controller;
    volatile uint32_t *registers = static_cast<volatile uint32_t*>(controller.GetIoArea());

    for (size_t i = 0; i < Controller200::count; ++i)
    {
        EXPECT_EQ(0xDEADBEEFu ^ i, registers[i]);
    }
}
