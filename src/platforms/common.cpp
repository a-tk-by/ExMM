#include "common.hpp"

ExMM::BreakPointData::BreakPointData(): IoSpace(), Controller(), Offset(), Active(false)
{
}

void ExMM::BreakPointData::Set(ExMM::IoSpace* ioSpace)
{
    IoSpace = ioSpace;
    Controller = nullptr;
    Offset = 0;
    Active = true;
}

void ExMM::BreakPointData::Set(ExMM::IoSpace* ioSpace, ExMM::ControllerInterface* controller, size_t offset)
{
    IoSpace = ioSpace;
    Controller = controller;
    Offset = offset;
    Active = true;
}

void ExMM::BreakPointData::Unset()
{
    IoSpace = nullptr;
    Controller = nullptr;
    Offset = 0;
    Active = false;
}
