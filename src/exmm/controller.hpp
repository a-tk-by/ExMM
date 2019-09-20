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

        struct FieldHelper
        {
            FieldHelper(volatile RegisterSetType* registers, size_t offset)
                : registers(registers), offset(offset), somethingMatched(false)
            {}

            template<class F>
            FieldHelper& Case(volatile F RegisterSetType::* field, const std::function<void(volatile F&)>& callback)
            {
                const RegisterSetType* x = nullptr;
                const auto* ptr = &(x->*field);
                if (reinterpret_cast<size_t>(ptr) == offset)
                {
                    if (callback)
                    {
                        callback(registers->*field);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F>
            FieldHelper& Case(volatile F RegisterSetType::* field, const std::function<void(volatile RegisterSetType*, volatile F&)>& callback)
            {                
                if (SameField(field, offset))
                {
                    if (callback)
                    {
                        callback(registers, registers->*field);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            void Else(std::function<void()> callback) const
            {
                if (!somethingMatched)
                {
                    callback();
                }
            }

        private:
            volatile RegisterSetType* registers;
            size_t offset;

            bool somethingMatched;

            template<class F>
            static bool SameField(volatile F RegisterSetType::* field, size_t offset)
            {
                const RegisterSetType* x = nullptr;
                const auto* ptr = &(x->*field);
                return reinterpret_cast<size_t>(ptr) == offset;
            }
        };

        FieldHelper SwitchField(RegisterSetType* data, size_t offset)
        {
            return FieldHelper(data, offset);
        }

        virtual void HookRead(RegisterSetType* data, size_t offset)
        {}

        virtual void HookWrite(RegisterSetType* data, size_t offset)
        {}

        virtual void Initialize(RegisterSetType* data)
        {}

        volatile RegisterSetType* GetIoArea() const
        {
            return publicIoArea;
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
            IoSpace *ioSpace = Registry::Add(this, sizeof(RegisterSetType), HookType);
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
            IoSpace *ioSpace = Registry::Add(this, size, HookType);

            this->publicIoArea = reinterpret_cast<RegisterSetType*>(ioSpace->GetPublicArea());
            this->privateIoArea = reinterpret_cast<RegisterSetType*>(ioSpace->GetPrivateArea());
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

        RegisterSetType* publicIoArea;
        RegisterSetType* privateIoArea;

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
