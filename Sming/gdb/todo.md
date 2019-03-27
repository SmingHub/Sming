
Console and File I/O
--------------------

This is a potential future enhancement to the gdbstub. See GDB manual for full details.

	"The File I/O remote protocol extension allows the target to use the host’s file system and console I/O to perform various system calls. System calls on the target system are translated into a remote protocol packet to the host system, which then performs the needed actions and returns a response packet to the target system. This simulates file system operations even on targets that lack fle systems."

As an example, we might implement a function in gdbstub called `gdb_console_read`:

```
char buffer[32];
int len = gdb_console_read(buffer, sizeof(buffer));
m_printf("gdb_console_read() returned %d\n", len);
```

This pauses the user application whilst waiting for input from the GDB console. Hitting return ends the call and the data is stored in buffer. Note: it _may_ be possible to have the application continue executing while the system call is in progress.

The same approach can be used to write to the console, to read/write host files or to make system calls. This could help speed application development by using files from the host, instead of a local file system (e.g. SPIFFS).
