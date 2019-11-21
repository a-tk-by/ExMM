#ifndef _EXMM_CONTROLLER_H_
#define _EXMM_CONTROLLER_H_

#include <functional>
#include <map>
#include <mutex>
#include <cstddef>

#include "hooktypes.hpp"
#include "controllerinterface.hpp"
#include "registry.hpp"

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

            template<class F, class Func>
            FieldHelper& Case(F Registers::* volatile field, const Func& callback)
            {
                if (SameField(offset, field))
                {
                   callback(registers->*field);
                   somethingMatched = true;
                }
                return *this;
            }

            template<class F, class Func>
            FieldHelper& Inside(F Registers::* volatile field, const Func& callback)
            {
                size_t nestedOffset;
                if (InsideField(offset, field, nestedOffset))
                {
                    volatile F* ptr = &(registers->*field);
                    FieldHelper<F> next(ptr, nestedOffset);
                    callback(next);

                    somethingMatched = true;
                }
                return *this;
            }

            template<class F, std::size_t N, class Func>
            FieldHelper& CaseArray(F(Registers::* volatile field)[N], const Func& callback)
            {
                std::size_t index;
                if (SameField(offset, field, index))
                {
                    callback(index, (registers->*field)[index]);
                    somethingMatched = true;
                }
                return *this;
            }

            template<class F, std::size_t N, class Func>
            FieldHelper& InsideArray(F(Registers::* volatile field)[N], const Func& callback)
            {
                std::size_t index;
                size_t nestedOffset;

                if (InsideField(offset, field, index, nestedOffset))
                {
                    volatile F* ptr = &(registers->*field)[index];
                    FieldHelper<F> next = FieldHelper<F>(ptr, nestedOffset);
                    callback(index, next);

                    somethingMatched = true;
                }
                return *this;
            }

            template <class Func>
            void Else(const Func& callback) const
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
