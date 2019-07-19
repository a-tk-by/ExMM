#ifndef _EXMM_PLATFORM_H_
#define _EXMM_PLATFORM_H_
#include "controller.hpp"

namespace ExMM
{
    enum class HookTypes;

    struct IoSpace
    {
        virtual void* GetPublicArea() = 0;
        virtual void* GetPrivateArea() = 0;
        virtual size_t Size() const = 0;

        virtual void Unprotect() = 0;
        virtual void RestoreProtection() = 0;

        virtual ~IoSpace() = default;
    };

    class Platform
    {
    public:
        static IoSpace* AllocateIoSpace(size_t size, ExMM::HookTypes hookTypes);
        static void RegisterHandlers();

        static void InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace);
        static void InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, struct ControllerInterface* controller, size_t offset);
        static bool GetBreakPoint(void* context, IoSpace*& ioSpace, ControllerInterface*& controller, size_t& offset);
        static void UninstallBreakPoint(void* context);
    };
}

#endif // _EXMM_PLATFORM_H_
