#if !defined(_WIN32) && defined(__x86_64__)

#include "posix-common.hpp"

#include <signal.h>
#include <stdio.h>

bool ExMM::Posix::IsMemoryWriteAccess(void* _context)
{
    ucontext_t* context = reinterpret_cast<ucontext_t*>(_context);
    printf("%08x\n", context->uc_mcontext.gregs[REG_ERR]);
    return context->uc_mcontext.gregs[REG_ERR] & 2;
}

void* ExMM::Posix::GetInstructionAddress(void* arg)
{
    return arg;
}

#endif
