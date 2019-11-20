#pragma once
#ifndef _POSIX_COMMON_HPP_
#define _POSIX_COMMON_HPP_

#if !defined(_WIN32)

namespace ExMM {
namespace Posix {

bool IsMemoryWriteAccess(void* context);
void* GetInstructionAddress(void* context);

}}

#endif
#endif 
