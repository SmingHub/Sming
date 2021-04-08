GDB Stub for Host
=================

This defines the command line to use when ``make gdb`` is run. No additional code is required to debug for the Host.

If you want to debug your application while having a separate UART then make sure to send the following commands to your debugger::

   handle SIGUSR1 nostop noprint

This component provides also ``gdbinit`` file containing the optimal settings needed for debugging.
