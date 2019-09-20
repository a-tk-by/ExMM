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
                if (SameField(field, offset))
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

            template<class F, std::size_t N>
            FieldHelper& Case(volatile F (RegisterSetType::* field)[N], const std::function<void(std::size_t index, volatile F&)>& callback)
            {
                std::size_t index;
                if (SameField(field, offset, index))
                {
                    if (callback)
                    {
                        callback(index, (registers->*field)[index]);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F, std::size_t N>
            FieldHelper& Case(volatile F RegisterSetType::* field, const std::function<void(volatile RegisterSetType*, std::size_t index, volatile F&)>& callback)
            {
                std::size_t index;
                if (SameField(field, offset, index))
                {
                    if (callback)
                    {
                        callback(registers, index, (registers->*field)[index]);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            void Else(std::function<void(std::size_t offset)> callback) const
            {
                if (!somethingMatched)
                {
                    callback(offset);
                }
            }

        private:
            volatile RegisterSetType* registers;
            std::size_t offset;

            bool somethingMatched;

            template<class F>
            static bool SameField(volatile F RegisterSetType::* field, std::size_t offset)
            {
                const RegisterSetType* x = nullptr;
                const auto* ptr = &(x->*field);
                return reinterpret_cast<size_t>(ptr) == offset;
            }

            template<class F, int N>
            static bool SameField(volatile F (RegisterSetType::* field)[N], std::size_t offset, std::size_t &index)
            {
                const RegisterSetType* x = nullptr;
                const auto* ptr = &(x->*field);

                const auto from = reinterpret_cast<size_t>(ptr);
                const auto to = from + sizeof(F) * N;

                if (offset >= from && offset < to)
                {
                    const auto diff = offset - from;
                    if (diff % sizeof(F) == 0)
                    {
                        index = diff / sizeof(F);
                        return true;
                    }
                }

                return false;
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

        void TriggerInterrupt(int vector = 0)
        {
            std::function<void()> routine;
            if (LookupInterruptHandler(vector, routine)) return;

            if (routine)
            {
                std::lock_guard<std::recursive_mutex> guard(interruptEntranceMutex);
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
            std::lock_guard<std::recursive_mutex> guard(interruptEntranceMutex);
            HookWrite(reinterpret_cast<RegisterSetType*>(data), offset);
        }

        void DoHookRead(void* data, size_t offset) override
        {
            std::lock_guard<std::recursive_mutex> guard(interruptEntranceMutex);
            HookRead(reinterpret_cast<RegisterSetType*>(data), offset);
        }

        void DoInitialize(void* data) override
        {
            Initialize(reinterpret_cast<RegisterSetType*>(data));
        }

        RegisterSetType* publicIoArea;
        RegisterSetType* privateIoArea;

        std::recursive_mutex interruptEntranceMutex;
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
