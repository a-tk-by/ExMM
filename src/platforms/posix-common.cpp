#if !defined(_WIN32)


#include "../exmm/platform.hpp"
#include "posix-iospace.hpp"
#include "posix-common.hpp"
#include "common.hpp"

#include <signal.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

static void CallOldHandler(struct sigaction& action, int sig, siginfo_t* info, void* context, bool kill)
{
    if (action.sa_flags & SA_SIGINFO)
    {
        if (action.sa_sigaction)
        {
            action.sa_sigaction(sig, info, context);
        }
    }
    else
    {
        if (action.sa_handler == SIG_IGN) return;
        if (action.sa_handler == SIG_DFL)
        {
            if (kill) exit(1);
            return;
        }
        if (action.sa_handler)
        {
            action.sa_handler(sig);
        }
    }
}

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

    if (!ExMM::Registry::FindController(reinterpret_cast<void*>(info->si_addr), controller, ioSpace, offset))
    {
        CallOldHandler(oldAccessHandler, sig, info, context, true);
        return;
    }

    if (ExMM::Posix::IsMemoryWriteAccess(context))
    {
        ioSpace->Unprotect();
        ExMM::Platform::InstallBreakPoint(context, ExMM::Posix::GetInstructionAddress(context), ioSpace, controller, offset);
        return;
    }
    else
    {
        controller->DoHookRead(ioSpace->GetPrivateArea(), offset);
        ioSpace->Unprotect();
        ExMM::Platform::InstallBreakPoint(context, ExMM::Posix::GetInstructionAddress(context), ioSpace);
        return;
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
    if (sigaction(SIGSEGV, &av_action, &oldAccessHandler) < 0)
    {
        throw std::runtime_error("Cannot register SISEGV handler");
    }

    struct sigaction tp_action = {};
    tp_action.sa_sigaction = TracePointHandler;
    tp_action.sa_flags = SA_SIGINFO;
    struct sigaction tp_old;
    if (sigaction(SIGTRAP, &tp_action, &tp_old) < 0)
    {
        throw std::runtime_error("Cannot register SIGTRAP handler");
    }
}

thread_local static BreakPointData breakPointData;

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace)
{
    breakPointData.Set(ioSpace);
    reinterpret_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_EFL] |= 0x100;
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, ControllerInterface* controller,
    size_t offset)
{
    breakPointData.Set(ioSpace, controller, offset);
    reinterpret_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_EFL] |= 0x100;
}

bool ExMM::Platform::GetBreakPoint(void* context, IoSpace*& ioSpace, ControllerInterface*& controller, size_t& offset)
{
    (void)context;
    if (breakPointData.Active)
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
    breakPointData.Unset();
    reinterpret_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_EFL] &=~ 0x100;
}

void ExMM::Platform::Run(const std::function<void()>& function)
{
    if (!function) return;
    
    function();
}


#endif
