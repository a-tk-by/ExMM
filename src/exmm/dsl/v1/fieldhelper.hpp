#pragma once
#ifndef _FIELD_HELPER_HPP_
#define _FIELD_HELPER_HPP_

#include <cstddef>

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
    static bool SameField(std::size_t offset, volatile F(Registers::* field)[N], std::size_t &index)
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

#endif
