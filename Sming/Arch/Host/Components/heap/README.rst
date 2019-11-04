Heap
====

This Component implements heap-related housekeeping functions. Heap usage is tracked using :component:`malloc_count`.
This also provides some validation (using *sentinels* to detect if memory blocks are overwritten).

.. envvar:: ENABLE_MALLOC_COUNT

   We require :component:`malloc_count` to keep track of heap usage for system_get_free_heap_size().
   It does this by hooking the memory allocation routines (malloc, free, etc.).
   If you wish to disable this behaviour, set `ENABLE_MALLOC_COUNT=0`.
   If using tools such as `Valgrind <https://www.valgrind.org>`__, this will provide a cleaner trace.
