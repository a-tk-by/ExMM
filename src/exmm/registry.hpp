#ifndef _EXMM_REGISTRY_
#define _EXMM_REGISTRY_

#include <cstddef>

namespace ExMM
{
    enum class HookTypes;

    struct ControllerInterface;

    class Registry
    {
    public:
        Registry() = delete;
        ~Registry()= delete;
        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) = delete;

        static void Remove(struct ControllerInterface* controller);
        static void* Add(ControllerInterface* controller, size_t size, HookTypes hookTypes);
        static bool FindController(void* rawData, ControllerInterface*& controller, struct IoSpace*& ioSpace, size_t& offset);
    };
}
#endif
