#ifndef _CONTROLLER_INTERFACE_H_
#define _CONTROLLER_INTERFACE_H_

#include <cstddef>
#include "hooktypes.hpp"

namespace ExMM
{
    struct ControllerInterface
    {
        virtual ~ControllerInterface() = default;
        virtual void DoHookRead(volatile void* data, size_t offset) = 0;
        virtual void DoHookWrite(volatile void* data, size_t offset) = 0;
        virtual HookTypes GetHookTypes() = 0;
    protected:
        ControllerInterface() = default;
        ControllerInterface(const ControllerInterface&) = default;
        ControllerInterface(ControllerInterface&&) = default;
        ControllerInterface& operator=(const ControllerInterface&) = default;
        ControllerInterface& operator=(ControllerInterface&&) = default;
    };

}

#endif
