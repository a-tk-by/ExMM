#if !defined(_WIN32)

#include "posix-iospace.hpp"

#include <sys/mman.h>
#include <sys/types.h>
#include <stdexcept>
#include <unistd.h>
#include <errno.h>
#include <string.h>


ExMM::Posix::IoSpace::IoSpace(size_t size, HookTypes hookTypes) 
    : hookTypes(hookTypes), size(size),
      file(-1), privateArea(), publicArea()
{
}

ExMM::IoSpace* ExMM::Posix::IoSpace::Initialize()
{
    char buffer[] = "/tmp/exmm-XXXXXX";
    file = mkstemp(buffer);
    if (file < 0) throw std::runtime_error("Cannot create file for mapping");
    
    if (ftruncate(file, size) < 0) throw std::runtime_error("Cannot set mapping file size");

    unlink(buffer);

    privateArea = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    if (errno) 
    {
	privateArea = nullptr;
	throw std::runtime_error(strerror(errno));
    }

    int publicFlags = PROT_READ | PROT_WRITE;
    if (hookTypes & HookTypes::Read) publicFlags &=~ PROT_READ;
    if (hookTypes & HookTypes::Write) publicFlags &=~ PROT_WRITE;
    publicArea = mmap(nullptr, size, publicFlags, MAP_SHARED, file, 0);
    if (errno) 
    {
	publicArea = nullptr;
	throw std::runtime_error(strerror(errno));
    }

    return this;
}

ExMM::Posix::IoSpace::~IoSpace()
{
    if (publicArea != nullptr)
    {
	munmap(publicArea, size);
    }
    if (privateArea != nullptr)
    {
	munmap(privateArea, size);
    }
    if (file >= 0)
    {
	close(file);
	file = -1;
    }
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
