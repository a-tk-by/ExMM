#ifndef _EXMM_PLATFORM_H_
#define _EXMM_PLATFORM_H_

#include <functional>
#include <cstddef>

#include "controllerinterface.hpp"

namespace ExMM
{
    struct IoSpace
    {
        virtual void* GetPublicArea() = 0;
        virtual void* GetPrivateArea() = 0;
        virtual size_t Size() const = 0;

        virtual void Unprotect() = 0;
        virtual void RestoreProtection() = 0;

        virtual ~IoSpace() = default;
    protected:
        IoSpace() = default;
    };

    class Platform
    {
    public:
        static struct IoSpace* AllocateIoSpace(size_t size, HookTypes hookTypes);
        static void RegisterHandlers();

        static void InstallBreakPoint(void* context, void* instruction, struct IoSpace* ioSpace);
        static void InstallBreakPoint(void* context, void* instruction, struct IoSpace* ioSpace, struct ControllerInterface* controller, size_t offset);
        static bool GetBreakPoint(void* context, IoSpace*& ioSpace, struct ControllerInterface*& controller, size_t& offset);
        static void UninstallBreakPoint(void* context);

        static void Run(const std::function<void()>& function);
    };
}

#endif // _EXMM_PLATFORM_H_
