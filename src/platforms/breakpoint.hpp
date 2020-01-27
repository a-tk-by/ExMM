#ifndef _PLATFORMS_COMMON_H_
#define _PLATFORMS_COMMON_H_
#include "../exmm/controllerinterface.hpp"
#include "../exmm/platform.hpp"

namespace ExMM
{
    struct EmptyPayload{};

    template<typename PayloadType = EmptyPayload>
    struct BreakPointData
    {
        ExMM::IoSpace* IoSpace;
        ExMM::ControllerInterface* Controller;
        size_t Offset;
        bool Active;
        PayloadType Payload;

        BreakPointData() : IoSpace(), Controller(), Offset(), Active(false), Payload() {}
        ~BreakPointData() = default;

        void Set(ExMM::IoSpace* ioSpace)
        {
            IoSpace = ioSpace;
            Controller = nullptr;
            Offset = 0;
            Active = true;
        }

        void Set(ExMM::IoSpace* ioSpace, ExMM::ControllerInterface* controller, size_t offset)
        {
            IoSpace = ioSpace;
            Controller = controller;
            Offset = offset;
            Active = true;
        }

        void Unset()
        {
            IoSpace = nullptr;
            Controller = nullptr;
            Offset = 0;
            Active = false;
        }


        BreakPointData(const BreakPointData&) = delete;
        BreakPointData(BreakPointData&&) = delete;
        BreakPointData& operator=(const BreakPointData&) = delete;
        BreakPointData& operator=(BreakPointData&&) = delete;

        static BreakPointData& Get()
        {
            thread_local static ExMM::BreakPointData<PayloadType> data;
            return data;
        }
    };
}
#endif // _PLATFORMS_COMMON_H_
