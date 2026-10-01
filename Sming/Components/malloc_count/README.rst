Malloc Count
============

This Component is a modified version of the original code at https://github.com/bingmann/malloc_count.
It is intended to provide basic heap monitoring for the Sming Host Emulator.
See :component-host:`heap`.

It is also supported on all architectures to provide more detailed heap allocation diagnostics.

.. note::

   Not supported for host builds on MacOS.

For a sample application see :samples:`Basic_Ssl`.

For Host builds, *malloc_count* is normally enabled.
Use :envvar:`ENABLE_MALLOC_COUNT` to control linkage.

To enable for use in a project, add **malloc_count** to :envvar:`COMPONENT_DEPENDS` in **component.mk**.


Basic usage
-----------

Measure allocation
   To establish RAM headroom for an application, use :cpp:func:`MallocCount::getCurrent` and :cpp:func:`MallocCount::getPeak`.

Measure heap activity
   Total accumulated heap allocation is returned by :cpp:func:`MallocCount::getTotal`.
   Call :cpp:func:`MallocCount::resetTotal` to clear.

   Total number of allocations is returned by :cpp:func:`MallocCount::getAllocCount`.

Restrict available RAM
   To test application low-memory behaviour use :cpp:func:`MallocCount::setAllocLimit`.
   Calling *malloc*, *new*, etc. will return *nullptr* if total allocation would exceed this value.

   .. note:: Host **ONLY**

      Calling :cpp:func:`system_get_free_heap_size` will reflect the configured limit.

Logging
   Enable debug console output with :cpp:func:`MallocCount::enableLogging`.
   By default, single allocations of 256 bytes or more are logged.

   Call :cpp:func:`MallocCount::setLogThreshold` to change this value.



Internal operation
------------------

When enabled, standard (architecture-dependent) memory allocation routines are intercepted.
Note that for *each* allocation an additional 16 bytes is consumed for internal use.

Part of this contains a fixed *sentinel* value which indicates that the memory block was actually allocated by *malloc_count*.

When *malloc_count* frees a memory block, it checks this sentinel value and, with logging enabled, outputs a *!!! memory corruption?* message.

This may indicate memory corruption, which the developer should investigate further: see :envvar:`ENABLE_SANITIZERS`.

Another explanation for this is that code somehow bypassed the *malloc_count* allocator.
This indicates an issue with *malloc_count* itself and an issue should be raised.


API Documentation
-----------------

.. doxygennamespace:: MallocCount
   :members:
