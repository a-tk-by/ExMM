#include "registry.hpp"
#include "platform.hpp"
#include <map>

namespace ExMM
{
    static Platform& GetPlatform()
    {
        static Platform platform;
        static bool initialized;
        if (!initialized)
        {
            platform.RegisterHandlers();
            initialized = true;
        }
        return platform;
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

    void* Registry::Add(ControllerInterface* controller, size_t size)
    {
        IoSpace* ioSpace = GetPlatform().AllocateIoSpace(size);
        ioMap.insert(std::make_pair(controller, ioSpace));
        return ioSpace->GetPublicArea();
    }

}
