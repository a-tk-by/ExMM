# Hooking mechanism
Memory protection mechanism implemented in operating systems like Windows/Linux/MacOS is being used to hook registers file of the controller's model. Nowadays, only x86 and x86_64 CPU architectures are supported.

Memory management unit (MMU) supports a memory protection mechanism for virtual memory's pages: it allows reading, writing and execution in different combinations, including none of them. When program incorrectly accesses protected memory area, MMU generated an exception which operating system may handle with its own way: to kill the process, to allocate physical memory page or translate into exception in process' context as SEH on Windows or as POSIX signal.
When process' exception occurs, it can be handled with some action and then program execution can be continued starting from the instruction that led the exception.

## Write hooking

When user's code writes value into the registers file memory protection failure occurs. This leads following sequence:
- Operating system calls error handler (SIGSEGV or EXCEPTION_ACCESS_VIOLATION). 
  - This handler determines, based on machine context if it was writing operation. 
  - It looks in the registry for a controller that owns the hooked memory area.
  - It modifies the thread's calling context to enable instruction tracing via flag TF in CPU register EFLAGS (x86/x86_64 only).
  - It resets registers file memory area protection to full access (both read and write operations are now enabled).
- The user's driver code continues from the instruction that led the access violation. So, write instruction will now be successfully completed.
- Trace exception occurs on instruction next after writing instruction and its handler is being called by the operating system (SIGTRAP or EXCEPTION_SINGLE_STEP).
  - This handler resets Trace flag (TF) in the thread CPU context.
  - It calls `HookWrite` method in the user's controller.
  - It sets original memory protection.
- The user's driver code continues from the instruction that triggered trace event. Now, user's code continues to work normally.

## Read hooking

When user's code read value from the registers file memory and read hooking is enables, protection failure occurs. This leads following sequence:
- Operating system call error handler (SEGSEGV or EXCEPTION_ACCESS_VIOLATION).
  - This handler determines, based on machine context if it was reading operation.
  - It looks in the registry for a controller that owns the hooked memory area.
  - It calls `HookRead` method in the user's controller.
  - It modifies the thread's calling context to enable instruction tracing via flag TF in CPU register EFLAGS (x86/x86_64 only).
  - It resets registers file memory area protection to full access (both read and write operations are now enabled).
- The user's driver code continues from the instruction that led the access violation. So, write instruction will now be successfully completed.
- Trace exception occurs on instruction next after writing instruction and its handler is being called by the operating system (SIGTRAP or EXCEPTION_SINGLE_STEP).
  - This handler resets Trace flag (TF) in the thread CPU context.
  - It sets original memory protection.
- The user's driver code continues from the instruction that triggered trace event. Now, user's code continues to work normally.
