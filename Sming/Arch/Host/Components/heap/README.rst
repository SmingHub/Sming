Heap
====

This Component implements heap-related housekeeping functions. Heap usage is tracked using :component:`malloc_count`.
This also provides some validation (using *sentinels* to detect if memory blocks are overwritten).
