#ifndef _EXMM_CONTROLLER_H_
#define _EXMM_CONTROLLER_H_

#include "registry.hpp"
#include "platform.hpp"
#include <functional>
#include <map>
#include <mutex>

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
    class ControllerBase : ControllerInterface
    {
    public:
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

        void ConnectInterruptHandler(int vector, std::function<void()> callback)
        {
            std::lock_guard<std::mutex> guard(interruptsMutex);
            interrupts[vector] = callback;
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

        void TriggerInterrupt(int vector)
        {
            std::function<void()> routine;
            if (LookupInterruptHandler(vector, routine)) return;

            if (routine)
            {
                std::lock_guard<std::mutex> guard(interruptEntranceMutex);
                routine();
            }
        }

    private:
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

        RegisterSetType* ioSpace;
        
        std::mutex interruptEntranceMutex;
        std::map<int, std::function<void()>> interrupts;
        std::mutex interruptsMutex;

        bool LookupInterruptHandler(int vector, std::function<void()>& routine)
        {
            std::lock_guard<std::mutex> guard(interruptsMutex);

            const auto handler = interrupts.find(vector);
            if (handler == interrupts.end())
            {
                return true;
            }

            routine = handler->second;
            return false;
        }

    };
}
#endif // _EXMM_CONTROLLER_H_
