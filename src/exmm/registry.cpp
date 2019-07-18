#include "registry.hpp"
#include "platform.hpp"
#include <map>

namespace ExMM
{
    static void InitPlatform()
    {
        static bool initialized;
        if (!initialized)
        {
            Platform::RegisterHandlers();
            initialized = true;
        }
    }

    static std::map<ControllerInterface*, IoSpace*> ioMap;

    void Registry::Remove(ControllerInterface* controller)
    {
        const auto mapElement = ioMap.find(controller);
        if (mapElement != ioMap.end())
        {
            delete mapElement->second;
            ioMap.erase(mapElement);
        }
    }

    void* Registry::Add(ControllerInterface* controller, size_t size, ExMM::HookTypes hookTypes)
    {
        InitPlatform();
        IoSpace* ioSpace = Platform::AllocateIoSpace(size, hookTypes);
        ioMap.insert(std::make_pair(controller, ioSpace));
        return ioSpace->GetPublicArea();
    }

}
