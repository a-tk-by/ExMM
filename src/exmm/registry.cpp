#include "registry.hpp"
#include "platform.hpp"

#include <cstddef>
#include <map>
#include <mutex>

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
    static std::mutex ioMapMutex;

    void Registry::Remove(ControllerInterface* controller)
    {
        std::lock_guard<std::mutex> guard(ioMapMutex);

        const auto mapElement = ioMap.find(controller);
        if (mapElement != ioMap.end())
        {
            delete mapElement->second;
            ioMap.erase(mapElement);
        }
    }

    struct IoSpace* Registry::Add(ControllerInterface* controller, size_t size, ExMM::HookTypes hookTypes)
    {
        InitPlatform();

        std::lock_guard<std::mutex> guard(ioMapMutex);

        IoSpace* ioSpace = Platform::AllocateIoSpace(size, hookTypes);
        ioMap.insert(std::make_pair(controller, ioSpace));
        return ioSpace;
    }

    bool Registry::FindController(void* rawData, ControllerInterface*& controller, IoSpace*& ioSpace, size_t& offset)
    {
        std::lock_guard<std::mutex> guard(ioMapMutex);
        char* data = reinterpret_cast<char*>(rawData);

        for (auto& item : ioMap)
        {
            char* first = reinterpret_cast<char*>(item.second->GetPublicArea());
            char* last = first + item.second->Size();

            if (data >= first && data < last)
            {
                ioSpace = item.second;
                controller = item.first;
                offset = data - first;
                return true;
            }
        }

        ioSpace = nullptr;
        controller = nullptr;
        return false;
    }
}
