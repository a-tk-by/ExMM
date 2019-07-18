#ifndef _EXMM_PLATFORM_H_
#define _EXMM_PLATFORM_H_
#include <memory>

namespace ExMM
{
    struct IoSpace
    {
        virtual void* GetPublicArea() = 0;
        virtual void* GetPrivateArea() = 0;
        virtual ~IoSpace() = default;
    };

    class Platform
    {
    public:
        IoSpace* AllocateIoSpace(size_t size);
        void RegisterHandlers();
    };
}

#endif // _EXMM_PLATFORM_H_
