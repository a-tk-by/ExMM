#ifndef _EXMM_CONTROLLER_H_
#define _EXMM_CONTROLLER_H_

#include "registry.hpp"
#include "platform.hpp"

namespace ExMM
{
    enum class HookTypes
    {
        None,
        Read = 1,
        Write = 2,
        ReadWrite = Read | Write
    };

    constexpr bool operator& (HookTypes lhs, HookTypes rhs)
    {
        return !!(static_cast<int>(lhs) & static_cast<int>(rhs));
    }

    struct ControllerInterface
    {
        virtual ~ControllerInterface() = default;
        virtual void DoHookRead(void* data, size_t offset) = 0;
        virtual void DoHookWrite(void* data, size_t offset) = 0;
        virtual HookTypes HookTypes() = 0;
    protected:
        ControllerInterface() = default;

        static Platform& GetPlatform();
    };

    template<HookTypes HookType, typename RegisterSetType = void>
    class ControllerBase : private ControllerInterface
    {
    public:
        ExMM::HookTypes HookTypes() override
        {
            return HookType;
        }

        void DoHookWrite(void* data, size_t offset) override
        {
            HookWrite(reinterpret_cast<RegisterSetType*>(data), offset);
        }

        void DoHookRead(void* data, size_t offset) override
        {
            HookRead(reinterpret_cast<RegisterSetType*>(data), offset);
        }

        virtual void HookRead(RegisterSetType* data, size_t offset)
        {}

        virtual void HookWrite(RegisterSetType* data, size_t offset)
        {}

        RegisterSetType* GetIoSpace() const
        {
            return ioSpace;
        }

        virtual ~ControllerBase()
        {
            Registry::Remove(this);
        }

        ControllerBase(const ControllerBase&) = delete;
        ControllerBase(ControllerBase&&) = delete;
        ControllerBase& operator=(const ControllerBase&) = delete;
        ControllerBase& operator=(ControllerBase&&) = delete;

    protected:
        template<size_t size = sizeof(RegisterSetType)>
        ControllerBase()
        {
            ioSpace = reinterpret_cast<RegisterSetType*>(Registry::Add(this, sizeof(RegisterSetType)));
        }

        explicit ControllerBase(size_t size)
        {
            ioSpace = reinterpret_cast<RegisterSetType*>(Registry::Add(this, sizeof(RegisterSetType)));
        }

    private:
        RegisterSetType* ioSpace;

    };
}
#endif // _EXMM_CONTROLLER_H_
