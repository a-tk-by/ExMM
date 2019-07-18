#include "windows-common.hpp"
#include <stdexcept>
#ifdef _WINDOWS

#include "Windows.h"

struct WindowsIoSpace : public ExMM::IoSpace
{
    WindowsIoSpace(size_t size)
        : hMapping(INVALID_HANDLE_VALUE), publicArea(nullptr), privateArea(nullptr)
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
            return;
        }

        publicArea = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
        privateArea = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);

    }

    ~WindowsIoSpace()
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

    HANDLE hMapping;

    void* publicArea;
    void* GetPublicArea() override
    {
        if (publicArea == nullptr)
        {
            throw std::runtime_error("Cannot create IO space");
        }
        return publicArea;
    }

    void* privateArea;
    void* GetPrivateArea() override
    {
        if (privateArea == nullptr)
        {
            throw std::runtime_error("Cannot create IO space");
        }
        return privateArea;
    }
};

ExMM::IoSpace* ExMM::Platform::AllocateIoSpace(size_t size)
{
    return new WindowsIoSpace(size);
}

void ExMM::Platform::RegisterHandlers()
{
}


#endif
