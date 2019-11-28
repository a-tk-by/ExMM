# Known problems
1. On Linux when accessing memory address not managed by ExMM process kills itself by calling function `exit(1)`.
2. Bad DSL performance due to instantiating callbacks on each call of `HookRead`/`HookWrite`.
