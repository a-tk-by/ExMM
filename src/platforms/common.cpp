#include "common.hpp"

BreakPointData::BreakPointData(): IoSpace(), Controller(), Offset(), Active(false)
{
}

void BreakPointData::Set(ExMM::IoSpace* ioSpace)
{
    IoSpace = ioSpace;
    Controller = nullptr;
    Offset = 0;
    Active = true;
}

void BreakPointData::Set(ExMM::IoSpace* ioSpace, ExMM::ControllerInterface* controller, size_t offset)
{
    IoSpace = ioSpace;
    Controller = controller;
    Offset = offset;
    Active = true;
}

void BreakPointData::Unset()
{
    IoSpace = nullptr;
    Controller = nullptr;
    Offset = 0;
    Active = false;
}
