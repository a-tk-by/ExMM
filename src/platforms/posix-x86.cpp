#if !defined(_WIN32) && defined(__i386__) && !defined(__x86_64__)

#include "posix-common.hpp"

#include <signal.h>

bool ExMM::Posix::IsMemoryWriteAccess(void* _context)
{
    ucontext_t* context = reinterpret_cast<ucontext_t*>(_context);
    return context->uc_mcontext.gregs[REG_ERR] & 2;
}

void* ExMM::Posix::GetInstructionAddress(void* _context)
{
    ucontext_t* context = reinterpret_cast<ucontext_t*>(_context);
    return reinterpret_cast<void*>(context->uc_mcontext.gregs[REG_EIP]);
}

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

void ExMM::Platform::UninstallBreakPoint(void* context)
{
    breakPointData.Unset();
    reinterpret_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_EFL] &=~ 0x100;
}


#endif
