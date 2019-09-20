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

        template<class Registers>
        struct FieldHelper
        {
            FieldHelper(volatile Registers* registers, size_t offset)
                : registers(registers), offset(offset), somethingMatched(false)
            {}

            template<class F>
            FieldHelper& Case(volatile F Registers::* field, const std::function<void(volatile F&)>& callback)
            {
                if (SameField(offset, field))
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
            FieldHelper& Case(volatile F Registers::* field, const std::function<void(volatile Registers*, volatile F&)>& callback)
            {
                if (SameField(offset, field))
                {
                    if (callback)
                    {
                        callback(registers, registers->*field);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F>
            FieldHelper& Inside(volatile F Registers::* field, const std::function<void(volatile Registers*, FieldHelper<F>& next)>& callback)
            {
                size_t nestedOffset;
                if (InsideField(offset, field, nestedOffset))
                {
                    if (callback)
                    {
                        volatile F* ptr = &(registers->*field);
                        FieldHelper<F> next(ptr, nestedOffset);
                        callback(registers, next);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F>
            FieldHelper& Inside(volatile F Registers::* field, const std::function<void(FieldHelper& next)>& callback)
            {
                size_t nestedOffset;
                if (InsideField(offset, field, nestedOffset))
                {
                    if (callback)
                    {
                        volatile F* ptr = &(registers->*field);
                        FieldHelper<F> next(ptr, nestedOffset);
                        callback(next);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F, std::size_t N>
            FieldHelper& Case(volatile F(Registers::* field)[N], const std::function<void(std::size_t index, volatile F&)>& callback)
            {
                std::size_t index;
                if (SameField(offset, field, index))
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
            FieldHelper& Case(volatile F Registers::* field, const std::function<void(volatile Registers*, std::size_t index, volatile F&)>& callback)
            {
                std::size_t index;
                if (SameField(offset, field, index))
                {
                    if (callback)
                    {
                        callback(registers, index, (registers->*field)[index]);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F, std::size_t N>
            FieldHelper& Inside(volatile F(Registers::* field)[N], const std::function<void(std::size_t index, FieldHelper<F>& next)>& callback)
            {
                std::size_t index;
                size_t nestedOffset;

                if (InsideField(offset, field, index, nestedOffset))
                {
                    if (callback)
                    {
                        volatile F* ptr = &(registers->*field)[index];
                        FieldHelper<F> next = FieldHelper<F>(ptr, nestedOffset);
                        callback(index, next);
                    }
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F, std::size_t N>
            FieldHelper& Inside(volatile F (Registers::* field)[N], const std::function<void(volatile Registers*, std::size_t index, FieldHelper& next)>& callback)
            {
                std::size_t index;
                size_t nestedOffset;

                if (InsideField(offset, field, index, nestedOffset))
                {
                    if (callback)
                    {
                        volatile F* ptr = &(registers->*field);
                        FieldHelper<F> next = FieldHelper<F>(ptr, nestedOffset);
                        callback(registers, index, next);
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
            volatile Registers* registers;
            std::size_t offset;

            bool somethingMatched;

            template<class F>
            static bool SameField(std::size_t offset, volatile F Registers::* field)
            {
                const Registers* x = nullptr;
                const auto* ptr = &(x->*field);
                return reinterpret_cast<size_t>(ptr) - reinterpret_cast<size_t>(x) == offset;
            }

            template<class F>
            static bool InsideField(std::size_t offset, volatile F Registers::* field, std::size_t& nestedOffset)
            {
                const Registers* x = nullptr;
                const auto* ptr = &(x->*field);
                if ((reinterpret_cast<size_t>(ptr) - reinterpret_cast<size_t>(x) >= offset) &&
                    (reinterpret_cast<size_t>(ptr) - reinterpret_cast<size_t>(x) < offset + sizeof(F)))
                {
                    nestedOffset = reinterpret_cast<size_t>(ptr) - reinterpret_cast<size_t>(x) - offset;
                    return true;
                }
                return false;
            }

            template<class F, int N>
            static bool SameField(std::size_t offset, volatile F (Registers::* field)[N], std::size_t &index)
            {
                const Registers* x = nullptr;
                const auto* ptr = &(x->*field);

                const auto from = reinterpret_cast<size_t>(ptr) - reinterpret_cast<size_t>(x);
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

            template<class F, int N>
            static bool InsideField(std::size_t offset, volatile F(Registers::* field)[N], std::size_t &index, std::size_t& nestedOffset)
            {
                const Registers* x = nullptr;
                const auto* ptr = &(x->*field);

                const auto from = reinterpret_cast<size_t>(ptr) - reinterpret_cast<size_t>(x);
                const auto to = from + sizeof(F) * N;

                if (offset >= from && offset < to)
                {
                    const auto diff = offset - from;
                    index = diff / sizeof(F);
                    nestedOffset = diff % sizeof(F);
                    return true;
                }

                return false;
            }
        };

        FieldHelper<RegisterSetType> SwitchField(RegisterSetType* data, size_t offset)
        {
            return FieldHelper<RegisterSetType>(data, offset);
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
