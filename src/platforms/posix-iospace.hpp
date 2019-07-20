#if !defined(_WIN32)

#include "../exmm/platform.hpp"

namespace ExMM {
namespace Posix {
    
    struct IoSpace : public ExMM::IoSpace
    {
        IoSpace(size_t size, HookTypes hookTypes);

        ExMM::IoSpace* Initialize();

        void* GetPublicArea() override;
        void* GetPrivateArea() override;
        size_t Size() const override;
        void Unprotect() override;
        void RestoreProtection() override;

    private:
        HookTypes hookTypes;
        size_t size;
        void* privateArea;
        void* publicArea;
    };

}}


#endif
