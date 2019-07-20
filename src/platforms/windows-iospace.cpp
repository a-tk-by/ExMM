#if defined(_WIN32)

#include "windows-iospace.hpp"

ExMM::Windows::IoSpace::IoSpace(size_t size, ExMM::HookTypes hookTypes):
    size(size),
    hookTypes(hookTypes),
    hMapping(INVALID_HANDLE_VALUE),
    publicArea(nullptr),
    privateArea(nullptr),
    oldProtection()
{
}

ExMM::IoSpace* ExMM::Windows::IoSpace::Initialize()
{
    LARGE_INTEGER sizeCopy;
    sizeCopy.QuadPart = size;
    hMapping = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE | SEC_COMMIT,
        sizeCopy.HighPart, sizeCopy.LowPart,
        nullptr
    );

    if (hMapping == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Cannot create file mapping");
    }

    publicArea = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
    privateArea = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);

    if (publicArea == nullptr || privateArea == nullptr)
    {
        throw std::runtime_error("Cannot map memory area");
    }

    DWORD newProtection;
    switch (hookTypes)
    {
    case HookTypes::Read:
        newProtection = PAGE_NOACCESS;
        break;
    case HookTypes::Write:
        newProtection = PAGE_READONLY;
        break;
    case HookTypes::ReadWrite:
        newProtection = PAGE_NOACCESS;
        break;
    default:
        newProtection = PAGE_READWRITE;
        break;
    }

    DWORD oldProtection;
    if (!VirtualProtect(publicArea, size, newProtection, &oldProtection))
    {
        throw std::runtime_error("Cannot set memory area protection");
    }

    return this;
}

ExMM::Windows::IoSpace::~IoSpace()
{
    if (publicArea != nullptr)
    {
        UnmapViewOfFile(publicArea);
    }

    if (privateArea != nullptr)
    {
        UnmapViewOfFile(privateArea);
    }

    if (hMapping != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hMapping);
    }
}

void* ExMM::Windows::IoSpace::GetPublicArea()
{
    return publicArea;
}

void* ExMM::Windows::IoSpace::GetPrivateArea()
{
    return privateArea;
}

size_t ExMM::Windows::IoSpace::Size() const
{
    return size;
}

void ExMM::Windows::IoSpace::Unprotect()
{
    if (!VirtualProtect(publicArea, size, PAGE_READWRITE, &oldProtection))
    {
        throw std::runtime_error("Cannot unprotect memory area");
    }
}

void ExMM::Windows::IoSpace::RestoreProtection()
{
    DWORD dummy;
    if (!VirtualProtect(publicArea, size, oldProtection, &dummy))
    {
        throw std::runtime_error("Cannot restore protection of memory area");
    }
}

#endif
