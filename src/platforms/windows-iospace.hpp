#ifndef _WINDOWS_IOSPACE_H_
#define _WINDOWS_IOSPACE_H_

#if defined(_WIN32)

#include "../exmm/platform.hpp"
#include <Windows.h>

namespace ExMM {
namespace Windows {

    struct IoSpace final : public ExMM::IoSpace
    {
        IoSpace(size_t size, ExMM::HookTypes hookTypes) :
            size(size),
            hookTypes(hookTypes),
            hMapping(INVALID_HANDLE_VALUE),
            publicArea(nullptr),
            privateArea(nullptr),
            oldProtection()
        {
        }

        IoSpace(const IoSpace&) = delete;

        ExMM::IoSpace* Initialize()
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

        ~IoSpace()
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

        size_t size;
        ExMM::HookTypes hookTypes;

        HANDLE hMapping;

        void* publicArea;

        void* GetPublicArea() override
        {
            return publicArea;
        }

        void* privateArea;

        void* GetPrivateArea() override
        {
            return privateArea;
        }

        size_t Size() const override
        {
            return size;
        }

        DWORD oldProtection;

        void Unprotect() override
        {
            if (!VirtualProtect(publicArea, size, PAGE_READWRITE, &oldProtection))
            {
                throw std::runtime_error("Cannot unprotect memory area");
            }
        }

        void RestoreProtection() override
        {
            DWORD dummy;
            if (!VirtualProtect(publicArea, size, oldProtection, &dummy))
            {
                throw std::runtime_error("Cannot restore protection of memory area");
            }
        }
    };
}}

#endif //_WIN32
#endif //_WINDOWS_IOSPACE_H_