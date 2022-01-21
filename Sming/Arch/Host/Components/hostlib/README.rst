Host Library
============

This Components provides the core funcionality for the Host Emulator:

Sockets
-------

Classes to provide simple Berkeley socket support for both Linux and Windows

Options
-------

Command line argument parsing. Applications may pass parameters and access them using
the :cpp:class:`CommandLine`.

Startup
-------

Initialises :component-host:`spi_flash`, Uart server (in :component-host:`driver`) and :component-host:`lwip`
networking, then enters the main task loop. This loop services LWIP plus the task and timer queues
(implemented in :component-host:`esp_hal`).
The ``Ctrl+C`` keypress is trapped to provide an orderly exit. If the system has become stuck in a loop or is otherwise
unresponsive, subsequent Ctrl+C presses will force a process termination.

Threads and Interrupts
----------------------

The emulator uses Posix threads (pthread library) to perform background processing which would probably
be done in hardware or which is outside of the framework.

Ideally we'd use SCHED_FIFO to disable time-slicing and more closely resemble how interrupts work
on a single-core CPU. However, this mode isn't supported in Windows, and Linux requires privileged
access and can potentially crash the system. Best avoided, I think.

All ESP code runs at a specific interrupt level, where 0 represents regular code.
Interrupts are triggered from a separate thread (a CThread instance) which calls :cpp:func:`
When an interrupt
occurs, the level is raised according to the priority of that interrupt. Until that code has finished,
only interrupts of a higher priority will preempt it.

Thread 'interrupt' code is sandwiched between calls to `interrupt_begin()` and `interrupt_end()`,
which blocks interrupts from other threads at the same or lower level.
The threads aren't suspended but will block if they call `interrupt_begin()`.
However, the main thread (level 0) is halted to reflect normal interrupt behaviour.


.. envvar:: LWIP_SERVICE_INTERVAL

   Default: 2ms

   LWIP stack is serviced via polling, this determines the interval.


.. envvar:: HOST_PARAMETERS

   Set this value to pass additional parameters to a Host application.
   For example::
   
      make run HOST_PARAMETERS='param1=12 param2="parameter with spaces"'
      make run HOST_PARAMETERS="param1=12 param2=\"parameter with spaces\""

   See :sample:`Basic_Utility` for a worked example.


API
---

.. doxygenclass:: CommandLine
   :members:
