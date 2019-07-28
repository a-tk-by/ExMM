#ifndef _CONTROLLER_INTERFACE_H_
#define _CONTROLLER_INTERFACE_H_

#include <stddef.h>
#include "hooktypes.hpp"

namespace ExMM
{
    struct ControllerInterface
    {
        virtual ~ControllerInterface() = default;
        virtual void DoHookRead(void* data, size_t offset) = 0;
        virtual void DoHookWrite(void* data, size_t offset) = 0;
        virtual void DoInitialize(void* data) = 0;
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
