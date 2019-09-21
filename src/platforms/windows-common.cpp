#ifdef _WIN32

#include "../exmm/platform.hpp"
#include "../exmm/controller.hpp"
#include "../exmm/registry.hpp"

#include <Windows.h>
#include <cstddef>

#include "windows-iospace.hpp"
#include "common.hpp"

static const DWORD TRAP_FLAG_MASK = 0x100;

ExMM::IoSpace* ExMM::Platform::AllocateIoSpace(size_t size, ExMM::HookTypes hookTypes)
{
    return (new Windows::IoSpace(size, hookTypes))->Initialize();
}

static LONG WINAPI ExceptionHook(EXCEPTION_POINTERS* info);

static LPTOP_LEVEL_EXCEPTION_FILTER previousFilter;

void ExMM::Platform::RegisterHandlers()
{
    previousFilter = SetUnhandledExceptionFilter(ExceptionHook);
}

static LONG WINAPI ExceptionHook(EXCEPTION_POINTERS* info)
{
    switch (info->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    {
        struct ExMM::ControllerInterface* controller;
        size_t offset;
        struct ExMM::IoSpace* ioSpace;

        if (!ExMM::Registry::FindController(reinterpret_cast<void*>(info->ExceptionRecord->ExceptionInformation[1]),
            controller, ioSpace, offset))
        {
            // Not a memory managed by ExMM
            break;
        }

        switch (info->ExceptionRecord->ExceptionInformation[0])
        {
        case 0: // read memory
            controller->DoHookRead(ioSpace->GetPrivateArea(), offset);
            ioSpace->Unprotect();
            ExMM::Platform::InstallBreakPoint(info->ContextRecord, info->ExceptionRecord->ExceptionAddress, ioSpace);
            return EXCEPTION_CONTINUE_EXECUTION;

        case 1: // write memory
            ioSpace->Unprotect();
            ExMM::Platform::InstallBreakPoint(info->ContextRecord, info->ExceptionRecord->ExceptionAddress, ioSpace, controller, offset);
            return EXCEPTION_CONTINUE_EXECUTION;
        default:
            break;
        }
    }
    break;
    case EXCEPTION_SINGLE_STEP:
    {
        ExMM::IoSpace* ioSpace;
        ExMM::ControllerInterface* controller;
        size_t offset;

        if (ExMM::Platform::GetBreakPoint(info->ContextRecord, ioSpace, controller, offset))
        {
            if (controller && (controller->GetHookTypes() & ExMM::HookTypes::Write))
            {
                controller->DoHookWrite(ioSpace->GetPrivateArea(), offset);
            }

            ExMM::Platform::UninstallBreakPoint(info->ContextRecord);
            ioSpace->RestoreProtection();

            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    break;
    default:
        break;
    }

    if (previousFilter)
    {
        return previousFilter(info);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}


thread_local static ExMM::BreakPointData breakPointData;

static void InstallBreakPoint(void* context)
{
    reinterpret_cast<PCONTEXT>(context)->EFlags |= TRAP_FLAG_MASK; // Enable Trap Flag (TF) in EFlags register
}


void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace)
{
    (void)instruction;
    breakPointData.Set(ioSpace);
    ::InstallBreakPoint(context);
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, ControllerInterface* controller,
    size_t offset)
{
    (void)instruction;
    breakPointData.Set(ioSpace, controller, offset);
    ::InstallBreakPoint(context);
}

bool ExMM::Platform::GetBreakPoint(void* context, IoSpace*& ioSpace, ControllerInterface*& controller, size_t& offset)
{
    (void)context;
    if (::breakPointData.Active)
    {
        ioSpace = breakPointData.IoSpace;
        controller = breakPointData.Controller;
        offset = breakPointData.Offset;
        return true;
    }
    return false;
}

void ExMM::Platform::UninstallBreakPoint(void* context)
{
    reinterpret_cast<PCONTEXT>(context)->EFlags &= ~ TRAP_FLAG_MASK;
    breakPointData.Unset();
}

void ExMM::Platform::Run(const std::function<void()>& function)
{
    if (!function) return;

    __try
    {
        function();
    }
    __except (ExceptionHook(GetExceptionInformation()))
    {
    }
}


#endif
