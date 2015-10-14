This project is an example how to integrate GDB debugging into your project.
It relies on the GDBStub project to do the heavy-lifting.

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
ENABLE_GDB make
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
