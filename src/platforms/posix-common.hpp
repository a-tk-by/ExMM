#if !defined(_WIN32)

namespace ExMM {
namespace Posix {

bool IsMemoryWriteAccess(void* context);
void* GetInstructionAddress(void* context);

}}

#endif
