#if !defined(_WIN32)


#include "../exmm/platform.hpp"
#include "posix-iospace.hpp"

ExMM::IoSpace* ExMM::Platform::AllocateIoSpace(size_t size, ExMM::HookTypes hookTypes)
{
    return (new Posix::IoSpace(size, hookTypes))->Initialize();
}

void ExMM::Platform::RegisterHandlers()
{
    //TODO: handlers
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace)
{
    //TODO
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, ControllerInterface* controller,
    size_t offset)
{
    //TODO
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
