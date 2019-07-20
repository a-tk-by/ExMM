#if !defined(_WIN32)

#include "posix-iospace.hpp"
#include <sys/mman.h>

ExMM::Posix::IoSpace::IoSpace(size_t size, HookTypes hookTypes) 
    : hookTypes(hookTypes), size(size),
      privateArea(), publicArea()
{
}

ExMM::IoSpace* ExMM::Posix::IoSpace::Initialize()
{

    return this;
}

void* ExMM::Posix::IoSpace::GetPublicArea()
{
    return publicArea;
}

void* ExMM::Posix::IoSpace::GetPrivateArea()
{
    return privateArea;
}

size_t ExMM::Posix::IoSpace::Size() const
{
    return size;
}

void ExMM::Posix::IoSpace::Unprotect()
{
    //TODO
}

void ExMM::Posix::IoSpace::RestoreProtection()
{
    //TODO
}


#endif
