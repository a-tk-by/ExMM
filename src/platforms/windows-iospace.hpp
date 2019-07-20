#ifndef _WINDOWS_IOSPACE_H_
#define _WINDOWS_IOSPACE_H_

#if defined(_WIN32)

#include "../exmm/platform.hpp"
#include <Windows.h>
#include <cstddef>

namespace ExMM {
namespace Windows {

    struct IoSpace final : public ExMM::IoSpace
    {
        IoSpace(size_t size, ExMM::HookTypes hookTypes);

        IoSpace(const IoSpace&) = delete;

        ExMM::IoSpace* Initialize();

        virtual ~IoSpace();

        size_t size;
        ExMM::HookTypes hookTypes;

        HANDLE hMapping;

        void* publicArea;

        void* GetPublicArea() override;

        void* privateArea;

        void* GetPrivateArea() override;

        size_t Size() const override;

        DWORD oldProtection;

        void Unprotect() override;

        void RestoreProtection() override;
    };
}}

#endif //_WIN32
#endif //_WINDOWS_IOSPACE_H_