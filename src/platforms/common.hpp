#ifndef _PLATFORMS_COMMON_H_
#define _PLATFORMS_COMMON_H_
#include "../exmm/controllerinterface.hpp"
#include "../exmm/platform.hpp"

namespace ExMM
{
    struct BreakPointData
    {
        ExMM::IoSpace* IoSpace;
        ExMM::ControllerInterface* Controller;
        size_t Offset;
        bool Active;

        BreakPointData();

        void Set(ExMM::IoSpace* ioSpace);

        void Set(ExMM::IoSpace* ioSpace, ExMM::ControllerInterface* controller, size_t offset);

        void Unset();

        static BreakPointData& Get();
    };
}
#endif // _PLATFORMS_COMMON_H_
