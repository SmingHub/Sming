This project is an example of how to integrate GDB debugging into your project.
It relies on the GDBStub project to do the heavy-lifting.

Exception Handling
------------------
If there is an exception in your code usually ESP prints a message like the following one:

```
Fatal exception (0):
epc1=0x4020997c, epc2=0x00000000, epc3=0x00000000, excvaddr=0x00000000, depc=0x00000000
```

That information can help you discover the function call that caused the exception.
Using the value of epc1 and executing a command like the one below:

```bash
xtensa-lx106-elf-objdump -dtr out/build/app.out | grep 4020997c
```

can give you an idea about the function. In my test case this is:
```
4020997c:	fffe61        	l32r	a6, 40209974 <user_rf_pre_init+0x45c>
```

But that information might not be enough to find the issue. And finding the
root cause may take quite some time.

GDB Debugging
-------------
Debugging is a powerful technique giving better understanding of the code and
the things that went wrong.

There is already existing GDBStub that tries to make it easier to use software
debugger. And this project is an example of what you need to do in order to
integrate it.

Here are the commands that you need to execute:

1. Fetch the [GDBStub](https://github.com/espressif/esp-gdbstub) by
executing the following commands ( usually needs to be done only once).

```bash
git submodule init
git submodule update --recursive
```

2. You will need a version of the sming library with enabled GDBStub functionality.
For that purpose you should compile Sming with ENABLE_GDB flag. Under Linux
you should do the following:

```bash
cd <path-to-sming-code>/Sming
make clean
ENABLE_GDB=1 make
```

3. In you project inside of you Makefile-user.mk file you should add the following
variable:

```make
ENABLE_GDB=1
```

If you are looking for an example then take a look at the Makefile-user.mk file
that is in the same directory as this README.md file.

4. Now compile your project and flash it to the board.
```bash
make
make flash
```

5. Run gdb immediately after resetting the board or after it has run into an exception.
The easiest way to do it is to use the provided script:
```bash
xtensa-lx106-elf-gdb -x <path-to-sming-code->/Basic_Debug/gdbcmds -b 115200
```

115200 stands for the baud rate your program is using. Change it accordingly.
You may also need to change the gdbcmds script to fit the configuration of your hardware and build environment.

6. Software breakpoints ('br') only work on code that is in RAM. During development you can use the GDB_IRAM_ATTR attribute in your function declarations.
Code in flash can only have a hardware breakpoint ('hbr').

Read the [Notes](https://github.com/espressif/esp-gdbstub#notes) for more information.
