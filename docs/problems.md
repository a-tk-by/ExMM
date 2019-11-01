# Known problems
On Linux when accessing memory address not managed by ExMM: if SIGSEGV handler had value SIG_DFL when initializing library, process kills itself by calling function `exit(1)`.
