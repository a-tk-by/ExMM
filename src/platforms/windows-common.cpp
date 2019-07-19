#ifdef _WIN32

#include "../exmm/platform.hpp"
#include "../exmm/controller.hpp"

#include <Windows.h>

#include "windows-iospace.hpp"

// diStorm3 library is used to parse instructions stream
// https://github.com/gdabah/distorm
// It is mounted as submodule
#include "../../external/distorm/include/distorm.h"

#ifdef _WIN64
#define DISTORM_DECODE_TYPE Decode64Bits
#else
#define DISTORM_DECODE_TYPE Decode32Bits
#endif

ExMM::IoSpace* ExMM::Platform::AllocateIoSpace(size_t size, ExMM::HookTypes hookTypes)
{
    return (new Windows::IoSpace(size, hookTypes))->Initialize();
}

static LONG WINAPI ExceptionHook(EXCEPTION_POINTERS* info);

static LPTOP_LEVEL_EXCEPTION_FILTER previousFilter;

void ExMM::Platform::RegisterHandlers()
{
    previousFilter = SetUnhandledExceptionFilter(ExceptionHook);
}

static LONG WINAPI ExceptionHook(EXCEPTION_POINTERS* info)
{
    switch (info->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    {
        ExMM::ControllerInterface* controller;
        size_t offset;
        ExMM::IoSpace* ioSpace;

        if (!ExMM::Registry::FindController(reinterpret_cast<void*>(info->ExceptionRecord->ExceptionInformation[1]),
            controller, ioSpace, offset))
        {
            // Not a memory managed by ExMM
            break;
        }

        switch (info->ExceptionRecord->ExceptionInformation[0])
        {
        case 0: // read memory
            controller->DoHookRead(ioSpace->GetPrivateArea(), offset);
            ioSpace->Unprotect();
            ExMM::Platform::InstallBreakPoint(info->ContextRecord, info->ExceptionRecord->ExceptionAddress, ioSpace);
            return EXCEPTION_CONTINUE_EXECUTION;

        case 1: // write memory
            ioSpace->Unprotect();
            ExMM::Platform::InstallBreakPoint(info->ContextRecord, info->ExceptionRecord->ExceptionAddress, ioSpace, controller, offset);
            return EXCEPTION_CONTINUE_EXECUTION;
        default:
            break;
        }
    }
    break;
    case EXCEPTION_SINGLE_STEP:
    {
        ExMM::IoSpace* ioSpace;
        ExMM::ControllerInterface* controller;
        size_t offset;

        if (ExMM::Platform::GetBreakPoint(info->ContextRecord, ioSpace, controller, offset))
        {
            if (controller && (controller->HookTypes() & ExMM::HookTypes::Write))
            {
                controller->DoHookWrite(ioSpace->GetPrivateArea(), offset);
            }

            ExMM::Platform::UninstallBreakPoint(info->ContextRecord);
            ioSpace->RestoreProtection();

            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    break;
    default:
        break;
    }

    if (previousFilter)
    {
        return previousFilter(info);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

const unsigned ExMMBreakPointSignature = 'ExMM';

struct BreakPointData
{
    ExMM::IoSpace* IoSpace;
    ExMM::ControllerInterface* Controller;
    size_t Offset;

    BreakPointData(ExMM::IoSpace* ioSpace)
        : IoSpace(ioSpace), Controller(), Offset()
    {}

    BreakPointData(ExMM::IoSpace* ioSpace, ExMM::ControllerInterface* controller, size_t offset)
        : IoSpace(ioSpace), Controller(controller), Offset(offset)
    {}
};

void* SearchNextInstruction(void* _instruction)
{
    _DecodedInst instructions[2];
    unsigned used;
    auto instruction = static_cast<unsigned char*>(_instruction);

    distorm_decode(0, instruction, 16,  DISTORM_DECODE_TYPE, instructions, 2, &used);
    return instruction + instructions[0].size;
}

static void InstallBreakPoint(void* _context, void* instruction, BreakPointData* breakPointData)
{
    PCONTEXT context = reinterpret_cast<PCONTEXT>(_context);

    void* target = SearchNextInstruction(instruction);

    *reinterpret_cast<void**>(&context->Dr0) = target;
    *reinterpret_cast<void**>(&context->Dr1) = breakPointData;
    context->Dr2 = ExMMBreakPointSignature;

    context->Dr6 = 0;
    // Local Dr0 breakpoint is active (bits 0..1)
    // See https://en.wikipedia.org/wiki/X86_debug_register
    context->Dr7 = 1;
}


void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace)
{
    ::InstallBreakPoint(context, instruction, new BreakPointData(ioSpace));
}

void ExMM::Platform::InstallBreakPoint(void* context, void* instruction, IoSpace* ioSpace, ControllerInterface* controller,
    size_t offset)
{
    ::InstallBreakPoint(context, instruction, new BreakPointData(ioSpace, controller, offset));
}

bool ExMM::Platform::GetBreakPoint(void* _context, IoSpace*& ioSpace, ControllerInterface*& controller, size_t& offset)
{
    PCONTEXT context = reinterpret_cast<PCONTEXT>(_context);

    if (context->Dr2 == ExMMBreakPointSignature)
    {
        BreakPointData* data = reinterpret_cast<BreakPointData*>(context->Dr1);
        if (data)
        {
            ioSpace = data->IoSpace;
            controller = data->Controller;
            offset = data->Offset;
            return true;
        }
    }

    return false;
}

void ExMM::Platform::UninstallBreakPoint(void* _context)
{
    const PCONTEXT context = reinterpret_cast<PCONTEXT>(_context);

    if (context->Dr2 == ExMMBreakPointSignature)
    {
        BreakPointData* data = reinterpret_cast<BreakPointData*>(context->Dr1);
        delete data;

        context->Dr7 = 0;
        context->Dr6 = 0;

        context->Dr0 = 0;
        context->Dr1 = 0;
        context->Dr2 = 0;
        context->Dr3 = 0;
    }
}


#endif
