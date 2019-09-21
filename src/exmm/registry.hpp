#ifndef _EXMM_REGISTRY_
#define _EXMM_REGISTRY_

#include <cstddef>
#include "hooktypes.hpp"
#include "controllerinterface.hpp"
#include "platform.hpp"

namespace ExMM
{
    struct Registry
    {
        Registry() = delete;
        ~Registry()= delete;
        Registry(const Registry&) = delete;
        Registry(Registry&&) = delete;
        Registry& operator=(const Registry&) = delete;
        Registry& operator=(Registry&&) = delete;

        static void Remove(struct ControllerInterface* controller);
        static struct IoSpace* Add(struct ControllerInterface* controller, size_t size, ExMM::HookTypes hookTypes);
        static bool FindController(void* rawData, struct ControllerInterface*& controller, struct IoSpace*& ioSpace, size_t& offset);
    };
}
#endif
