#if defined(__x86_64__) && defined(__APPLE__)

#include <csignal>

#include "common.hpp"
#include "posix-common.hpp"
#include "../exmm/platform.hpp"

bool ExMM::Posix::IsMemoryWriteAccess(void* _context)
{
    auto context = reinterpret_cast<ucontext_t*>(_context);
    auto& mcontext = *context->uc_mcontext;
    return mcontext.__es.__err & 2;
}

void* ExMM::Posix::GetInstructionAddress(void* _context)
{
    auto context = reinterpret_cast<ucontext_t*>(_context);
    return reinterpret_cast<void*>(context->uc_mcontext->__ss.__rip);
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace)
{
    BreakPointData::Get().Set(ioSpace);
    reinterpret_cast<ucontext_t*>(context)->uc_mcontext->__ss.__rflags |= 0x100;
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, ControllerInterface* controller,
    size_t offset)
{
    BreakPointData::Get().Set(ioSpace, controller, offset);
    reinterpret_cast<ucontext_t*>(context)->uc_mcontext->__ss.__rflags |= 0x100;
}

void ExMM::Platform::UninstallBreakPoint(void* context)
{
    BreakPointData::Get().Unset();
    reinterpret_cast<ucontext_t*>(context)->uc_mcontext->__ss.__rflags &= ~0x100;
}


#endif
