#ifndef _HOOK_TYPES_H_
#define _HOOK_TYPES_H_

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
}

#endif
