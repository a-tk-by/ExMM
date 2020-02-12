#if !defined(_WIN32) && defined(__arm__) && !defined(__aarch64__)

#include <csignal>
#include <unistd.h>
#include <sys/mman.h>

#include "breakpoint.hpp"
#include "posix-common.hpp"
#include "../exmm/platform.hpp"

struct Bkpt
{
    void *Address;
    uint32_t Opcode;
    Bkpt() : Address(), Opcode() {}
    void Save(void* addr)
    {
        Opcode = *reinterpret_cast<uint32_t*>(addr);
        Address = addr;
    }

    void Restore()
    {
        if (Address)
        {
            *reinterpret_cast<uint32_t*>(Address) = Opcode;
            Address = 0;
            Opcode = 0;
        }
    }
};

bool ExMM::Posix::IsMemoryWriteAccess(void* _context)
{
    auto context = reinterpret_cast<ucontext_t*>(_context);
    return context->uc_mcontext.error_code & (1 << 11); // See FSR_WRITE macro in /arch/arm/mm/fault.h header in Linux repository
}

void* ExMM::Posix::GetInstructionAddress(void* _context)
{
    auto context = reinterpret_cast<ucontext_t*>(_context);
    return reinterpret_cast<void*>(context->uc_mcontext.arm_pc);
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace)
{
    InstallBreakPoint(context, instruction, ioSpace, nullptr, 0);
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, ControllerInterface* controller, size_t offset)
{
    auto& ctx = reinterpret_cast<ucontext_t*>(context)->uc_mcontext;
    size_t addr = ctx.arm_pc;
    
    void* ptr = (void*)addr;

    const auto page = getpagesize();
    auto base = addr - addr % page;
    mprotect((void*)base, page, PROT_READ | PROT_WRITE | PROT_EXEC);

    if (ctx.arm_cpsr & (1<<5)) // Thumb or Thumb-2 mode
    {
        auto* current = reinterpret_cast<uint16_t*>(addr);
        auto* next = current;
        switch (*current & 0xF800)
        {
           case 0xE800: case 0xF000: case 0xF800: // Thumb32 instruction
                next += 2;
                break;
            case 0xE000: // Thumb/branch instruction. Theoretically it is not possiple to trap this case.
            default: // normal Thumb instruction
                next += 1;
                break;
        }

        BreakPointData<Bkpt>::Get().Set(ioSpace, controller, offset).Payload.Save(next);
        *next = 0xBE00; // BKPT<0> instruction, same for both Thumb and Thumb2 ISA
    }
    else // ARM mode
    {
        auto* next = reinterpret_cast<uint32_t*>(addr) + 1;
        BreakPointData<Bkpt>::Get().Set(ioSpace, controller, offset).Payload.Save(next);
        *next = 0xE1200070; // BKPT<0> instruction
    }
}

void ExMM::Platform::UninstallBreakPoint(void* context)
{
    BreakPointData<Bkpt>::Get().Unset().Payload.Restore();
}

bool ExMM::Platform::GetBreakPoint(void* context, IoSpace*& ioSpace, ControllerInterface*& controller, size_t& offset)
{
    (void)context;
    auto& breakPointData = BreakPointData<Bkpt>::Get();
    if (breakPointData.Active)
    {
        ioSpace = breakPointData.IoSpace;
        controller = breakPointData.Controller;
        offset = breakPointData.Offset;
        return true;
    }
    return false;
}


#endif
