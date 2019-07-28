#ifndef _EXMM_CONTROLLER_H_
#define _EXMM_CONTROLLER_H_

#include <functional>
#include <map>
#include <mutex>
#include <cstddef>

#include "registry.hpp"

#include "hooktypes.hpp"
#include "controllerinterface.hpp"

namespace ExMM
{
    template<HookTypes HookType, typename RegisterSetType = void>
    class ControllerBase : ControllerInterface
    {
    public:
        virtual void HookRead(RegisterSetType* data, size_t offset)
        {}

        virtual void HookWrite(RegisterSetType* data, size_t offset)
        {}

        virtual void Initialize(RegisterSetType* data)
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
            ioSpace = reinterpret_cast<RegisterSetType*>(Registry::Add(this, sizeof(RegisterSetType), HookType));
        }

        template<typename = std::enable_if<std::is_same<RegisterSetType, void>::value>>
        explicit ControllerBase(size_t size)
        {
            ioSpace = Registry::Add(this, size, HookType);
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
        HookTypes GetHookTypes() override
        {
            return HookType;
        }

        void DoHookWrite(void* data, size_t offset) override
        {
            std::lock_guard<std::mutex> guard(interruptsMutex);
            HookWrite(reinterpret_cast<RegisterSetType*>(data), offset);
        }

        void DoHookRead(void* data, size_t offset) override
        {
            std::lock_guard<std::mutex> guard(interruptsMutex);
            HookRead(reinterpret_cast<RegisterSetType*>(data), offset);
        }

        void DoInitialize(void* data) override
        {
            Initialize(reinterpret_cast<RegisterSetType*>(data));
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
