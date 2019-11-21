#ifndef _WINDOWS_IOSPACE_H_
#define _WINDOWS_IOSPACE_H_

#if defined(_WIN32)

#include "../exmm/platform.hpp"
#include <Windows.h>
#include <cstddef>

namespace ExMM {
namespace Windows {

    class IoSpace final : public ExMM::IoSpace
    {
    public:
        IoSpace(size_t size, ExMM::HookTypes hookTypes);

        IoSpace(const IoSpace&) = delete;
        IoSpace& operator=(const IoSpace&) = delete;

        ExMM::IoSpace* Initialize();

        void* GetPublicArea() override;

        void* GetPrivateArea() override;

        size_t Size() const override;

        void Unprotect() override;

        void RestoreProtection() override;

        virtual ~IoSpace();
    private:
        size_t size;
        ExMM::HookTypes hookTypes;
        HANDLE hMapping;
        void* publicArea;
        void* privateArea;
        DWORD oldProtection;
    };
}}

#endif //_WIN32
#endif //_WINDOWS_IOSPACE_H_