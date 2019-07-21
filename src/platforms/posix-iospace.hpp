#if !defined(_WIN32)

#include "../exmm/platform.hpp"

namespace ExMM {
namespace Posix {
    
    struct IoSpace : public ExMM::IoSpace
    {
        IoSpace(size_t size, HookTypes hookTypes);

        ExMM::IoSpace* Initialize();

        virtual ~IoSpace();

        void* GetPublicArea() override;
        void* GetPrivateArea() override;
        size_t Size() const override;
        void Unprotect() override;
        void RestoreProtection() override;

    private:
        HookTypes hookTypes;
        size_t size;
        int file;
        void* privateArea;
        void* publicArea;
        int oldProtection;
    };

}}


#endif
