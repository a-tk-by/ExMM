#ifndef _EXMM_H_
#define _EXMM_H_

#include "exmm/controller.hpp"
#include "exmm/platform.hpp"

namespace ExMM
{
    
    inline void Run(const std::function<void()>& userCode)
    {
        Platform::Run(userCode);
    }
}

#endif // _EXMM_H_
