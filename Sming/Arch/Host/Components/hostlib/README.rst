Host Library
============

This Components provides the core funcionality for the Host Emulator:

Sockets
-------

Classes to provide simple Berkeley socket support for both Linux and Windows

Options
-------

Command line argument parsing

Startup
-------

Initialises :component-host:`spi_flash`, Uart server (in :component-host:`driver`) and :component-host:`lwip`
networking, then enters the main task loop. This loop services LWIP plus the task and timer queues
(implemented in :component-host:`esp_hal`).
The ``Ctrl+C`` keypress is trapped to provide an orderly exit. If the system has become stuck in a loop or is otherwise
unresponsive, subsquent Ctrl+C presses will force a process termination.

Threads and Interrupts
----------------------

The emulator uses Posix threads (pthread library) to perform background processing which would probably
be done in hardware or which is outside of the framework.

Ideally we'd use SCHED_FIFO to disable time-slicing and more closely resemble how interrupts work
on a single-core CPU. However, this mode isn't supported in Windows, and Linux requires priviledged
access and can potentially crash the system. Best avoided, I think.

All ESP code runs at a specific interrupt level, where 0 represents regular code. When an interrupt
occurs, the level is raised according to the priority of that interrupt. Until that code has finished,
only interrupts of a higher priority will pre-empt it.

The ``set_interrupt_level`` function is used to ensure that threads running at different interrupt
levels do not pre-empty each other, as this would introduce problems that do not exist on real hardware.
The main thread is also suspended during interrupt execution.
