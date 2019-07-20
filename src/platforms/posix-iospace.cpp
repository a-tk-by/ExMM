#if !defined(_WIN32)

#include "posix-iospace.hpp"
#include <sys/mman.h>

ExMM::Posix::IoSpace::IoSpace(size_t size, HookTypes hookTypes) 
    : hookTypes(hookTypes), size(size),
      privateArea(), publicArea()
{
}


#endif
