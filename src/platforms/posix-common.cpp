#if !defined(_WIN32)


#include "../exmm/platform.hpp"
#include "../exmm/registry.hpp"
#include "posix-iospace.hpp"
#include "posix-common.hpp"
#include "common.hpp"

#include <csignal>
#include <stdexcept>
#include <cstring>

ExMM::IoSpace* ExMM::Platform::AllocateIoSpace(size_t size, ExMM::HookTypes hookTypes)
{
    return (new Posix::IoSpace(size, hookTypes))->Initialize();
}

static struct sigaction oldAccessHandler;

static void AccessViolationHandler(int sig, siginfo_t* info, void* context)
{
    ExMM::ControllerInterface* controller;
    size_t offset;
    ExMM::IoSpace* ioSpace;

    if (!ExMM::Registry::FindController(info->si_addr, controller, ioSpace, offset))
    {
        exit(1);
        return;
    }

    if (ExMM::Posix::IsMemoryWriteAccess(context))
    {
        ioSpace->Unprotect();
        ExMM::Platform::InstallBreakPoint(context, ExMM::Posix::GetInstructionAddress(context), ioSpace, controller, offset);
    }
    else
    {
        controller->DoHookRead(ioSpace->GetPrivateArea(), offset);
        ioSpace->Unprotect();
        ExMM::Platform::InstallBreakPoint(context, ExMM::Posix::GetInstructionAddress(context), ioSpace);
    }
}

static void TracePointHandler(int sig, siginfo_t* info, void* context)
{
    ExMM::IoSpace* ioSpace;
    ExMM::ControllerInterface* controller;
    size_t offset;

    if (ExMM::Platform::GetBreakPoint(context, ioSpace, controller, offset))
    {
        if (controller && (controller->GetHookTypes() & ExMM::HookTypes::Write))
        {
            controller->DoHookWrite(ioSpace->GetPrivateArea(), offset);
        }

        ExMM::Platform::UninstallBreakPoint(context);
        ioSpace->RestoreProtection();
    }
}


void ExMM::Platform::RegisterHandlers()
{
    struct sigaction av_action = {};
    av_action.sa_sigaction = AccessViolationHandler;
    av_action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &av_action, nullptr) < 0)
    {
        throw std::runtime_error("Cannot register SISEGV handler");
    }
    if (sigaction(SIGBUS, &av_action, nullptr) < 0)
    {
        throw std::runtime_error("Cannot register SISEGV handler");
    }

    struct sigaction tp_action = {};
    tp_action.sa_sigaction = TracePointHandler;
    tp_action.sa_flags = SA_SIGINFO;
    struct sigaction tp_old = {0};
    if (sigaction(SIGTRAP, &tp_action, &tp_old) < 0)
    {
        throw std::runtime_error("Cannot register SIGTRAP handler");
    }
}

bool ExMM::Platform::GetBreakPoint(void* context, IoSpace*& ioSpace, ControllerInterface*& controller, size_t& offset)
{
    (void)context;
    BreakPointData& breakPointData = BreakPointData<>::Get();
    if (breakPointData.Active)
    {
        ioSpace = breakPointData.IoSpace;
        controller = breakPointData.Controller;
        offset = breakPointData.Offset;
        return true;
    }
    return false;
}

void ExMM::Platform::Run(const std::function<void()>& function)
{
    if (!function) return;

    function();
}


#endif
