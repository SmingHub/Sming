Host Library
============

This Components provides the core funcionality for the Host Emulator:

sockets
   Classes to provide simple Berkeley socket support for both Linux and Windows

options
   Command line argument parsing

startup
   Initialises :component-host:`spi_flash`, Uart server (in :component-host:`driver`) and :component-host:`lwip`
   networking, then enters the main task loop. This loop services LWIP plus the task and timer queues
   (implemented in :component-host:`esp_hal`).
   The ``Ctrl+C`` keypress is trapped to provide an orderly exit. If the system has become stuck in a loop or is otherwise
   unresponsive, subsquent Ctrl+C presses will force a process termination.
