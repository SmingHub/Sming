Debugging on Host
====================

Required tools and hardware
---------------------------

For ``MacOS``, GDB can be installed via ``brew`` but it requires code-signing.
This is not a trivial procedure and ``lldb`` is recommended.
This is installed with the standard xcode development tools.

For other development platforms the GNU debugger is required.
Make sure that you have the following executable in your PATH::

    gdb

No additional hardware is required.

Recompilation is required
-------------------------

In order to debug applications based on Sming Framework make sure that
you are using Sming version 3.8.0 or newer.

Compilation directives
~~~~~~~~~~~~~~~~~~~~~~

If you want to debug your application and the Sming Framework code make sure to
(re)compile it with :envvar:`ENABLE_GDB=1 <ENABLE_GDB>` directive::

   cd $SMING_HOME/../samples/LiveDebug
   make components-clean # -- Forces a rebuild of all Components
   make ENABLE_GDB=1

The commands above will re-compile Sming with debug symbols and
optimizations for debugging. These commands need to be executed once.

If you need to debug the LWIP TCP/IP stack, add :envvar:`ENABLE_LWIPDEBUG` when rebuilding::

   make ENABLE_GDB=1 ENABLE_LWIPDEBUG=1


Application
~~~~~~~~~~~

To use, (re)compile your application with the ENABLE_GDB option.
For this example we will use the :sample:`LiveDebug` sample application::

   cd $SMING_HOME/../samples/LiveDebug
   make ENABLE_GDB=1 # -- recompiles your application with debugging support

The next step is to start the debugger. This can be done with the command below::

   make gdb

For ``MacOS``, use lldb::

   make lldb

After that a new interactive debugging session will be started::

   Welcome to SMING!
   Type 'r' to run application

To start the execution of the application type `r` or `run`::

   (gdb) r
   Starting program: /x/Sming/samples/LiveDebug/out/Host/debug/firmware/app --flashfile=out/Host/debug/firmware/flash.bin --flashsize=4M --pause
   [Thread debugging using libthread_db enabled]
   Using host libthread_db library "/lib/i386-linux-gnu/libthread_db.so.1".
   [New Thread 0xf7bdcb40 (LWP 16428)]

   Welcome to the Sming Host emulator

   host_flashmem_init: Created blank "out/Host/debug/firmware/flash.bin", 4194304 bytes

   ...
   main: >> Starting Sming <<

You can pause the program execution by pressing `Ctrl-C`.  And work further using some further GDB commands. The next paragraph describes some of them.

GDB commands
------------

For ``MacOS``, please refer to https://lldb.llvm.org/index.html for LLDB usage instructions.

There are multiple commands supported in GDB and we will mention only some of them.

List current source code
~~~~~~~~~~~~~~~~~~~~~~~~

One possibility is to see the source code of the current line where the
execution has stopped. To achieve this you should type ``list`` in the gdb
console::

    (gdb) list
    102     }
    103 }
    104
    105 int main(int argc, char* argv[])
    106 {
    107     trap_exceptions();
    108
    109     host_printf("\nWelcome to the Sming Host emulator\n\n");
    110
    111     static struct {

Break the execution
~~~~~~~~~~~~~~~~~~~

This command will pause the debugger once it reaches a specific function
or line in the code. This is called ``breakpoint`` and can be set like this::

   (gdb) break blink
   Breakpoint 1 at 0x40105d4c: file app/application.cpp, line 66.

Continue the execution
~~~~~~~~~~~~~~~~~~~~~~

To continue the execution of the application we can use the ``continue``
command::

   (gdb) continue
   Continuing.

   Breakpoint 1, blink () at app/application.cpp:66
   66 {
   (gdb)

Because we have set already a breakpoint for the ``blink`` function the
execution will be paused when the ``blink`` function is reached and from
here you can go to the next line or see the current values of the
variables.

Go to the next line
~~~~~~~~~~~~~~~~~~~

This can be done using ``next``::

   (gdb) next
   67      digitalWrite(LED_PIN, ledState);

See variable value
~~~~~~~~~~~~~~~~~~

The command to see a value is ``print`` followed by the name of the
value. For example to see the value of the ``ledState`` variable inside
the ``blink`` function we could type::

   (gdb) print ledState
   $1 = true

You can see more useful commands :ref:`here <useful-gdb-commands>`.

Or watch the following short video

.. image:: https://img.youtube.com/vi/hVwSX_7Ey8c/3.jpg
   :target: https://www.youtube.com/watch?v=hVwSX_7Ey8c
