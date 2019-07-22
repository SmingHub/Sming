Esp8266 GDBSTUB for Sming
=========================

Background
----------

This is a rewrite of gdbstub based on the
`esp8266 Arduino project <https://github.com/esp8266/Arduino/pull/5559>`__.

To use the GNU Debugger (GDB) with Sming requires your application to
include some code (``gdbstub``) which communicates via the serial port.
On the ESP8266 only UART0 may be used for this as UART1 is
transmit-only.

The gdbstub code will only be built if you specify :c:macro:`ENABLE_GDB`
=1 when compiling your application. At startup, before your init()
function is called, it will claim UART0 so your application will be
unable to use it directly. Therefore, the default port for ``Serial``
is changed to ``UART2``.

UART2 is a ‘virtual’ serial port to enable serial communications to work
correctly when GDB-enabled. Read/write calls and serial callbacks are
handled via gdbstub. Baud rate changes affect UART0 directly.

Note that ``target`` refers to the application being debugged, and
``host`` the development system running the GDB program.

Refer to the official
`GDB documention <https://sourceware.org/gdb/current/onlinedocs/gdb/index.html>`__
for further details.

GDB
---

This is the application which runs on your development system and talks
to ``gdbstub``.

-  Linux: A version of this should be available in
   ``$ESP_HOME/xtensa-lx106-elf/bin/xtensa-lx106-elf-gdb``

-  Windows: At time of writing, UDK doesn’t provide a GDB application
   - Download and run the executable installer at `SysProgs <http://gnutoolchains.com/esp8266/>`__

   - Copy the
     ``C:\SysGCC\esp8266\opt\xtensa-lx106-elf\bin\xtensa-lx106-elf-gdb.exe``
     to a suitable location.

-  Mac: ?

Usage
-----

-  Configure gdbstub by editing ``gdbstub-cfg.h`` as required. You
   can also configure the options by setting ::envvar:`USER_CFLAGS` in
   your project’s ``component.mk`` file. e.g
   ``USER_CFLAGS=-DGDBSTUB_BREAK_ON_INIT=0``.
-  Optional: Add ``gdb_do_break()`` statements to your application.
-  Run ``make clean``, then ``make ENABLE_GDB=1 flash`` to build and
   flash the application with debugging enabled
-  Run gdb, depending on your configuration immediately after
   resetting the board or after it has run into an exception. The
   easiest way to do it is to use the provided script: ``make gdb``.

To run manually in Linux:

.. code-block:: bash

   $ESP_HOME/xtensa-lx106-elf/bin/xtensa-lx106-elf-gdb -x $SMING_HOME/gdb/gdbcmds -b 115200 -ex "target remote /dev/ttyUSB0"

Windows command line:

.. code-block:: batch

   %ESP_HOME%\xtensa-lx106-elf\bin\xtensa-lx106-elf-gdb -x %SMING_HOME%\gdb\gdbcmds -b 115200 -ex "target remote COM4"

In both cases the appropriate baud rate and COM port should be
substituted.

Useful GDB commands
-------------------

``c`` Continue execution

``q`` Quit and detach

``where`` Display current stopped location

``bt`` Show stack backtrace

``disass`` Disassemble, ``disass/m`` to mix with source code

``print expr`` Display a variable or other value

``print func()`` Call a function, display result, or ``call func()`` to
discard result

``tui enable`` Provides a windowed interface within the console (only
seems to work in Linux)

``x/32xw $sp`` Display contents of stack

``info reg`` Display register values

``info break`` Display details of currently set breakpoints

``delete`` Delete all breakpoints

``br`` Set a breakpoint at the given address or function name

``hbr`` Set a hardware breakpoint

``watch`` Set a hardware watchpoint to detect when the value of a
variable changes

These commands require ``GDBSTUB_ENABLE_HOSTIO`` to be enabled:

``remote get targetfile hostfile`` Read a file from SPIFFS (on the
target)

``remote put hostfile targetfile`` Write a file to SPIFFS

``remote delete targetfile`` Delete a file from SPIFFS

Eclipse
-------

Windows:

-  Ensure ``Use external console for inferior`` is checked.
-  In connection settings, specify COM port like with leading /,
   e.g. \ ``/COM4``

Problems connecting?

-  Switch to the debug perspective before connecting
-  Ensure serial baud rate matches your application
-  Remove or disable all breakpoints before attaching. Eclipse will
   attempt to set these on connection, and if any are invalid it will
   hang and timeout.
-  Check connectivity using command-line GDB

GDB System Calls
----------------

Applications may interact with GDB directly using system calls, for
example reading input from the GDB command prompt. See the
:sample:`LiveDebug` sample for a demonstration.

Note that system calls are disabled in the default configuration, so set
:c:macro:`GDBSTUB_ENABLE_SYSCALL` =1 to use this feature with your
application.

Known Issues and Limitations
----------------------------

- Unable to set requested break/watch points
   - Cause: Due to hardware limitations, only one hardware
     breakpount and one hardware watchpoint are available
   - Solution: None (hardware limitation)

- System crashes if debugger is paused for too long
   - Cause: The WiFi hardware is designed to be serviced by
     software periodically. It has some buffers so it will behave OK
     when some data comes in while the processor is busy, but these
     buffers are not infinite. If the WiFi hardware receives lots of
     data while the debugger has stopped the CPU, it is bound to crash.
     This will happen mostly when working with UDP and/or ICMP;
     TCP-connections in general will not send much more data when the
     other side doesn’t send any ACKs.
   - Solution: In such situations avoid pausing the debugger for
     extended periods

- Software breakpoints/watchpoints (‘break’ and ‘watch’) don’t work on flash code
   - Cause: GDB handles these by replacing code with a debugging
     instruction, therefore the code must be in RAM.
   - Solution: Use hardware breakpoint (‘hbreak’) or use
     :c:macro:`GDB_IRAM_ATTR` for code which requires testing

- If hardware breakpoint is set, single-stepping won’t work unless code is in RAM.
   - Cause: GDB reverts to software breakpoints if no hardware
     breakpoints are available
   - Solution: Delete hardware breakpoint before single-stepping

- Crash occurs when setting breakpoint in HardwareTimer callback routine
   - Cause: By default, HardwareTimer uses Non-maskable Interrupts
     (NMI) which keep running when the debugger is paused
   - Solution: Use the timer in non-maskable mode, or enable
     :c:macro:`GDBSTUB_PAUSE_HARDWARE_TIMER` option

- If gdbstub isn’t initialised then UART2 won’t work, though initialisation will succeed
   - Cause: By design, uart callbacks can be registered for UART2
     at any time, before or after initialisation
   - Solution: Not really an issue, just something to be aware of

- Error reported, “packet reply is too long”
   - Cause: Mismatch between GDB version and stub code
   - Solution: Set :c:macro:`GDBSTUB_GDB_PATCHED` =1 or use an
     unpatched version of GDB

- Whilst GDB is attached, input cannot be passed to application
   - Cause: GDB buffers keystrokes and replays them only when the
     target is interrupted (e.g. via ctrl+C), rather than passing them
     via serial connection.
   - Solution: Application may use gdb_syscall interface to
     communicate with debugger. See
     ``$(SMING_HOME)/system/gdb_syscall.h``, and :sample:`LiveDebug`
     sample.

- No apparent way to have second ‘console’ (windows terminology) separate from GDB interface
   - Cause: Unknown
   - Solution: Is this possible with remote targets?

- GDB (in Windows) doesn’t respond at all to Ctrl+C
   - Cause: Unknown
   - Solution: Press Ctrl+Break to ‘hard kill’ GDB. You'll probably
     need to do the next step as well to get it back

- When GDB is running under windows, appears to hang when target reset or restarted
   - Cause: Unknown, may not happen on all devboards but presents
     with NodeMCU
   - Solution
      - quit GDB ``quit``
      - Start terminal ``make terminal``
      - reset board
      - quit terminal
      - run GDB again ``make gdb``

- Debug messages don’t appear in Eclipse
   - Cause: Unknown
   - Solution: Use command-line GDB, or a better visual debugger


Configuration
-------------

.. doxygenfile:: gdbstub-cfg.h

