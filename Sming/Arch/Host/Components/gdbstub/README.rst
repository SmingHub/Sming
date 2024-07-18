GDB Stub for Host
=================

This defines command lines to run Host debuggers, either the GNU debugger::

   make gdb

Or LLVM debugger::

   make lldb

Generally the GNU debugger is used but for MacOS lldb is the default.

This Component also provides default settings for each in the ``gdbcmds`` and ``lldbcmds`` files.

See :doc:`/debugging/host/index`.
