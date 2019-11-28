#ifndef _EXMM_CONTROLLER_H_
#define _EXMM_CONTROLLER_H_

#include <functional>
#include <map>
#include <mutex>
#include <cstddef>

#include "hooktypes.hpp"
#include "controllerinterface.hpp"
#include "registry.hpp"

#include "dsl/v1/fieldhelper.hpp"

namespace ExMM
{
    template<HookTypes HookType, typename RegisterSetType = void>
    class ControllerBase : ControllerInterface
    {
    public:

        FieldHelper<RegisterSetType> SwitchField(volatile RegisterSetType* data, size_t offset)
        {
            return FieldHelper<RegisterSetType>(data, offset);
        }

        virtual void HookRead(volatile RegisterSetType* data, size_t offset)
        {}

        virtual void HookWrite(volatile RegisterSetType* data, size_t offset)
        {}

        virtual void Initialize(volatile RegisterSetType* data)
        {}

        volatile RegisterSetType* GetIoArea() const
        {
            return publicIoArea;
        }

        virtual ~ControllerBase()
        {
            Registry::Remove(this);
        }

        void ConnectInterruptHandler(int vector, const std::function<void()>& callback)
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
            struct IoSpace *ioSpace = Registry::Add(this, sizeof(RegisterSetType), HookType);
            publicIoArea = reinterpret_cast<RegisterSetType*>(ioSpace->GetPublicArea());
            privateIoArea = reinterpret_cast<RegisterSetType*>(ioSpace->GetPrivateArea());
        }

        volatile RegisterSetType* GetPrivateIoArea() const
        {
            return privateIoArea;
        }

        template<typename = std::enable_if<std::is_same<RegisterSetType, void>::value>>
        explicit ControllerBase(size_t size)
        {
            struct IoSpace *ioSpace = Registry::Add(this, size, HookType);

            this->publicIoArea = reinterpret_cast<RegisterSetType*>(ioSpace->GetPublicArea());
            this->privateIoArea = reinterpret_cast<RegisterSetType*>(ioSpace->GetPrivateArea());
        }

        void TriggerInterrupt(int vector = 0)
        {
            std::function<void()> routine;
            if (LookupInterruptHandler(vector, routine)) return;

            if (routine)
            {
                std::lock_guard<std::recursive_mutex> guard(memoryLockMutex);
                routine();
            }
        }

        std::recursive_mutex& AcquireMemoryLock()
        {
            return this->memoryLockMutex;
        }

    private:
        HookTypes GetHookTypes() override
        {
            return HookType;
        }

        void DoHookWrite(volatile void* data, size_t offset) override
        {
            std::lock_guard<std::recursive_mutex> guard(memoryLockMutex);
            HookWrite(reinterpret_cast<volatile RegisterSetType*>(data), offset);
        }

        void DoHookRead(volatile void* data, size_t offset) override
        {
            std::lock_guard<std::recursive_mutex> guard(memoryLockMutex);
            HookRead(reinterpret_cast<volatile RegisterSetType*>(data), offset);
        }

        void DoInitialize(volatile void* data) override
        {
            Initialize(reinterpret_cast<volatile RegisterSetType*>(data));
        }

        RegisterSetType* publicIoArea;
        RegisterSetType* privateIoArea;

        std::recursive_mutex memoryLockMutex;

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
