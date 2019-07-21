#if !defined(_WIN32)


#include "../exmm/platform.hpp"
#include "posix-iospace.hpp"
#include "posix-common.hpp"
#include <signal.h>
#include <stdexcept>
#include <sys/ptrace.h>
#include <sys/user.h>
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
    printf("SigSegv handler\n");

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
        printf("Write access\n");
        ioSpace->Unprotect();
        ExMM::Platform::InstallBreakPoint(context, ExMM::Posix::GetInstructionAddress(context), ioSpace, controller, offset);
        return;
    }
    else
    {
        printf("Read access\n");
        controller->DoHookRead(ioSpace->GetPrivateArea(), offset);
        ioSpace->Unprotect();
        ExMM::Platform::InstallBreakPoint(context, ExMM::Posix::GetInstructionAddress(context), ioSpace);
        return;
    }
}

static struct sigaction oldTracePointHandler;
static void TracePointHandler(int sig, siginfo_t* info, void* context)
{
    printf("SigTrap handler\n");
    CallOldHandler(oldTracePointHandler, sig, info, context, false);
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
    if (sigaction(SIGTRAP, &tp_action, &oldTracePointHandler) < 0)
    {
        throw std::runtime_error("Cannot register SIGTRAP handler");
    }


    printf("Hello world\n");
    getchar();

}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace)
{
    printf("Install breakpoint\n");
    getchar();
    ptrace(PTRACE_POKEUSER, getpid(), offsetof(struct user, u_debugreg[0]), instruction);
    if (errno)
    {
        throw std::runtime_error(strerror(errno));
    }
    printf("Install breakpoint done\n");
    getchar();
    exit(1);
    //TODO
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, ControllerInterface* controller,
    size_t offset)
{
    printf("Install breakpoint\n");
    getchar();
    ptrace(PTRACE_POKEUSER, getpid(), offsetof(struct user, u_debugreg[0]), instruction);
    if (errno)
    {
        throw std::runtime_error(strerror(errno));
    }
    printf("Install breakpoint done\n");
    getchar();
    exit(1);    //TODO
}

bool ExMM::Platform::GetBreakPoint(void* _context, IoSpace*& ioSpace, ControllerInterface*& controller, size_t& offset)
{
    //TODO
    throw nullptr;
}

void ExMM::Platform::UninstallBreakPoint(void* _context)
{
    //TODO
}

void ExMM::Platform::Run(const std::function<void()>& function)
{
    if (!function) return;
    
    function();
}


#endif
