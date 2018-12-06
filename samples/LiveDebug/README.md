This project is an example of how to integrate GDB debugging into your project.
It relies on the GDBStub project to do the heavy-lifting.

Exception Handling
------------------
Sming comes with a built-in exception handling that takes care to display the stack trace 
leading to the issue. Usually it looks like this

```
***** Fatal exception 28
pc=0x40100e96 sp=0x3ffff640 excvaddr=0x000015b8
ps=0x00000033 sar=0x00000018 vpri=0x000000f0
r00: 0x40100d69=1074793833 r01: 0x3ffff640=1073739328 r02: 0x3fff3900=1073690880 
r03: 0x2b265ed4= 723934932 r04: 0x3fffbff0=1073725424 r05: 0x000015b8=      5560 
r06: 0x000015b8=      5560 r07: 0x14a8433b= 346571579 r08: 0x00000008=         8 
r09: 0x14a842f3= 346571507 r10: 0x3fff22d0=1073685200 r11: 0x00000003=         3 
r12: 0x00000048=        72 r13: 0x3fff38c0=1073690816 r14: 0x3ffe9da0=1073651104 
r15: 0x3fff1138=1073680696 

Stack dump:
To decode the stack dump call from command line:
   python $SMING_HOME/../tools/decode-stacktrace.py out/build/app.out
and copy & paste the text enclosed in '===='.
================================================================
3ffff640:  40100e96 00000033 00000018 000000f0  
3ffff650:  40100d69 3fff3900 2b265ed4 3fffbff0  
3ffff660:  000015b8 000015b8 14a8433b 00000008  
3ffff670:  14a842f3 3fff22d0 00000003 00000048  
3ffff680:  3fff38c0 3ffe9da0 3fff1138 0000001c  
3ffff690:  002222fb c0fb5c01 0bc10000 facfd1fb  
3ffff6a0:  620020c0 6162802d 0020c004 59062c52  
3ffff6b0:  0020c051 61492c48 210020c0 7c38fb50 
...  

================================================================
```

With the help of `decode-stacktrace.py` you can decode the stack trace to something readable like:

```
0x40100e96: pvPortRealloc at ??:                                                                                                                                                 ?
0x40100d69: pvPortMalloc at ??:?
0x402455f0: ax_port_malloc at C:\tools\Sming-3.1.2\Sming/third-party/axtls-8266/                                                                                                                                                 replacements/mem.c:51
0x4024561a: ax_port_calloc at C:\tools\Sming-3.1.2\Sming/third-party/axtls-8266/                                                                                                                                                 replacements/mem.c:63
0x40230acc: x509_new at c:\tools\Sming-3.1.2\Sming\third-party\axtls-8266/ssl/x5                                                                                                                                                 09.c:81
0x4023d3e4: m_vsnprintf at C:\tools\Sming-3.1.2\Sming/system/m_printf.cpp:69
0x4023d4a6: m_vprintf at C:\tools\Sming-3.1.2\Sming/system/m_printf.cpp:83
0x40000a39: ?? ??:0
0x4021418a: pp_attach at ??:?
0x40221d60: pbuf_alloc at ??:?
0x40221f0a: pbuf_copy at ??:?
0x4023d3e4: m_vsnprintf at C:\tools\Sming-3.1.2\Sming/system/m_printf.cpp:69
```

Using the information about the type of the exception (ex: `***** Fatal exception 28`)
and the sequence of commands might help us figure out the issue.

But that information might not be enough. And finding the
root cause may take quite some time.

GDB Debugging
-------------
Debugging is a powerful technique allowing you to interactively run your code and be able to see much more information about the things that went wrong.

There is already existing GDBStub that tries to make it easier to use software
debugger. And this project is an example of what you need to do in order to 
integrate it.

Here are the commands that you need to execute:

1. You will need a version of the Sming library with enabled GDBStub functionality.
For that purpose you should compile Sming with ENABLE_GDB flag. Under Linux
you should do the following:

```bash
cd $SMING_HOME
make dist-clean
ENABLE_GDB=1 make
```

2. In your project inside of your Makefile-user.mk file you should add the following
variable:

```make
ENABLE_GDB=1
```

If you are looking for an example then take a look at the Makefile-user.mk file 
that is in the same directory as this README.md file.

3. Now compile your project and flash it to the board.
```bash
make ENABLE_GDB=1 
make flash
```

4. Run gdb immediately after resetting the board or after it has run into an exception. 
The easiest way to do it is to use the provided script: 
```bash
xtensa-lx106-elf-gdb -x <path-to-sming-code->/Basic_Debug/gdbcmds -b 115200 
```

115200 stands for the baud rate your program is using. Change it accordingly.
You may also need to change the gdbcmds script to fit the configuration of your hardware and build environment.

5. Software breakpoints ('br') only work on code that is in RAM. During development you can use the GDB_IRAM_ATTR attribute in your function declarations. 
Code in flash can only have a hardware breakpoint ('hbr').

Read the [Notes](https://github.com/espressif/esp-gdbstub#notes) for more information.
