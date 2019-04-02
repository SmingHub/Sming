GDBSTUB for Sming
=================

Background
----------

This is a rewrite of gdbstub based on the [esp8266 Arduino project](https://github.com/esp8266/Arduino/pull/5559).

To use the GNU Debugger (GDB) with Sming requires your application to include some code (`gdbstub`) which communicates via the serial port. On the ESP8266 only UART0 may be used for this as UART1 is transmit-only.

The gdbstub code will only be built if you specify `ENABLE_GDB=1` when compiling your application. At startup, before your init() function is called, it will claim UART0 so your application will be unable to use it directly. Therefore, the default port for `Serial` is changed to `UART2`.

UART2 is a 'virtual' serial port to enable serial communications to work correctly when GDB-enabled. Read/write calls and serial callbacks are handled via gdbstub. Baud rate changes affect UART0 directly.

Note that `target` refers to the application being debugged, and `host` the development system running the GDB program.

Refer to the official [GDB documention](https://sourceware.org/gdb/current/onlinedocs/gdb/index.html) for further details.


GDB
---

This is the application which runs on your development system and talks to `gdbstub`.

 * Linux: A version of this should be available in $ESP_HOME/xtesnsa-lx106-elf/bin/xtensa-lx106-elf-gdb

 * Windows: At time of writing, UDK doesn't provide a GDB application. You can find pre-built version of this at [SysProgs](http://gnutoolchains.com/esp8266/). Download and run the executable installer, then copy the `C:\SysGCC\esp8266\opt\xtensa-lx106-elf\bin\xtensa-lx106-elf-gdb.exe` to a suitable location.

 * Mac: ?

Usage
-----

 * Configure gdbstub by editing `gdbstub-cfg.h` as required. You can also configure the options by setting `USER_CFLAGS` in your project's `Makefile-user.mk` file. e.g `USER_CFLAGS=-DGDBSTUB_BREAK_ON_INIT=0`.
 * Optional: Add `gdb_do_break()` statements to your application.
 * Run `make clean`, then `make ENABLE_GDB=1 flash` to build and flash the application with debugging enabled
 * Run gdb, depending on your configuration immediately after resetting the board or after it has run into
an exception. The easiest way to do it is to use the provided script: xtensa-lx106-elf-gdb -x $SMING_HOME/gdb/gdbcmds -b 115200
Change the '115200' into the baud rate your code uses. You may need to change the `gdbcmds` script to fit the
configuration of your hardware and build environment.

Useful GDB commands
-------------------

`c` Continue execution
`q` Quit and detach
`bt` Show stack backtrace
`disass` Disassemble
`disass/m` Disassemble, mix with source code
`print expr` Display a variable or other value
`print func()` Call a function, display result
`call func()` Call a function, discard result
`tui enable` Provides a windowed interface within the console (only seems to work in Linux)

Eclipse
-------

Windows:

 * Ensure `Use external console for inferior` is checked.
 * In connection settings, specify COM port like with leading /, e.g. `/COM4`
 
Problems connecting?

 * Switch to the debug perspective before connecting
 * Ensure serial baud rate matches your application
 * Remove or disable all breakpoints before attaching. Eclipse will attempt to set these on connection, and if any are invalid it will hang and timeout.
 * Check connectivity using command-line GDB

Known Issues and Limitations
----------------------------

Unable to set requested break/watch points
- Cause: Due to hardware limitations, only one hardware breakpount and one hardware watchpoint are available
- Solution: None (hardware limitation)

System crashes if debugger is paused for too long
- Cause: The WiFi hardware is designed to be serviced by software periodically. It has some buffers so it will behave OK when some data comes in while the processor is busy, but these buffers are not infinite. If the WiFi hardware receives lots of data while the debugger has stopped the CPU, it is bound to crash. This will happen mostly when working with UDP and/or ICMP; TCP-connections in general will not send much more data when the other side doesn't send any ACKs.
- Solution: In such situations avoid pausing the debugger for extended periods


Software breakpoints/watchpoints ('break' and 'watch') don't work on flash code
- Cause: GDB handles these by replacing code with a debugging instruction, therefore the code must be in RAM.
- Solution: Use hardware breakpoint ('hbreak') or use GDB_IRAM_ATTR for code which requires testing

If hardware breakpoint is set, single-stepping won't work unless code is in RAM.
- Cause: GDB reverts to software breakpoints if no hardware breakpoints are available
- Solution: Delete hardware breakpoint before single-stepping


Crash occurs when setting breakpoint in HardwareTimer callback routine 
- Cause: By default, HardwareTimer uses Non-maskable Interrupts (NMI) which keep running when the debugger is paused
- Solution: Use the timer in non-maskable mode, or enable GDBSTUB_PAUSE_HARDWARE_TIMER option

If gdbstub isn't initialised then UART2 won't work, though initialisation will succeed
- Cause: By design, uart callbacks can be registered for UART2 at any time, before or after initialisation
- Solution: Not really an issue, just something to be aware of

Error reported, "packet reply is too long"
- Cause: Mismatch between GDB version and stub code
- Solution: Set `GDBSTUB_GDB_PATCHED=1` or use an unpatched version of GDB

Whilst GDB is attached, input cannot be passed to application
- Cause: GDB buffers keystrokes and replays them only when the target is interrupted (e.g. via ctrl+C), rather than passing them via serial connection.
- Solution: Application may use gdb_syscall interface to communicate with debugger. See `$(SMING_HOME)/system/gdb_syscall.h`.

No apparent way to have second 'console' (windows terminology) separate from GDB interface
- Cause: Unknown
- Solution: Is this possible with remote targets?
