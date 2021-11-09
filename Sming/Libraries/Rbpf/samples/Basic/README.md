# Tutorial 1

This tutorial is a starting point for working with the RIOT Femto-Containers
system. It shows what is required as toolchain and how to get started with
loadable applications based on rBPF and Femto-Containers.

## Goals

 - Toolchain setup and checks
 - Compiling a first Femto-Containers application
 - Adding the applications to a small RIOT instance
 - Inspecting the application content

## Steps

### Toolchain setup

This step describes the required toolchain and applications required to compile
and execute the tutorials presented here. At least required are a compiler and
standard library for the target RIOT platform and the LLVM/Clang compiler to
compile the Femto-Container code.

First make sure that it is possible to compile a RIOT application for your
target platform. Please follow the steps from the [RIOT
documentation](https://doc.riot-os.org/getting-started.html) to get started with
this. The toolchain setup can be verified by compiling a simple RIOT example
such as the
[Default](https://github.com/RIOT-OS/RIOT/tree/master/examples/default) or the
[Hello World](https://github.com/RIOT-OS/RIOT/tree/master/examples/hello-world)
example.

To compile the eBPF code for the Femto-Containers the following applications and
libraries are required:

- LLVM
- Clang
- Python 3.6 or higher
- [pyelftools](https://github.com/eliben/pyelftools)

Please ensure that these are installed and verify that llvm supports generating
eBPF code by executing:

```Console
$ llc --version
```

This must show "bpf" in the list of registered targets.

### Compiling the Femto-Container application

The code itself, as visible in the `increment.c` source file is only a single
functions. It receives the argument and interprets it as a pointer to a 64 bit
number. It increments the number and returns it.

The source code of the Femto-Container application is in the `fc` subdirectory
of the tutorial directory. Compiling the code is done with make:

```Console
$ make -C fc
```

This uses Clang and LLVM to compile the source code to an eBPF object file which
is then converted via a python application to a Femto-Container-specific format.
The resulting binary has the same name as the source file, but with `.bin` as
extension. In this case the `increment.c` file is compiled to `increment.o` and
converted to `increment.bin`. The `.o` file can be inspected using the binutils
tools such as objdump. The content of the Femto-Container bin file can be
inspected using the tooling in
[`dist/tools/rbpf`](https://github.com/bergzand/RIOT/tree/wip/bpf/dist/tools/rbpf).

### Compiling the RIOT example with the Femto-Container application

The example RIOT code in this tutorial is a minimum working example with
Femto-Containers. On boot it loads the Femto-Containers application and launches
the application to execute it. It prints both the input value and the output
value of the container. It also prints the return code of the virtual machine
execution itself, indicating whether the machine successfully executed.

The compiled RIOT application includes the binary Femto-Container application.
Via the RIOT blob file mechanism, the raw file is included in the build.

Compiling the RIOT application is done via:

```Console
$ make
```

By default the application is built for the `native` platform, meaning that it
can be executed as an application on a regular Linux system. It can be executed
with

```Console
$ make term
```

And should show output similar to:

```Console
$ make term
RIOT native interrupts/signals initialized.
LED_RED_OFF
LED_GREEN_ON
RIOT native board initialized.
RIOT native hardware initialization complete.

main(): This is RIOT! (Version: 2017.01-devel-23600-g90c82-wip/bpf)
All up, running the Femto-Container application now
Input to the VM: 0
Return code (expected 0): 0
Result of the VM: 1
```

As visible, the virtual machine application receives `0` as input value and
returns `1` back to the operating system. The return code of the virtual machine
is `0`, indicating a successful execution of the application.

### Inspecting the Femto-Container application

As mentioned before, the Femto-Container application binary can be inspected
with tools provided by RIOT. Using the `gen_rbf.py` tool in
[`dist/tools/rbpf`](https://github.com/bergzand/RIOT/tree/wip/bpf/dist/tools/rbpf)
shows the content of the application when using the `dump` subcommand:

```Console
$ ../RIOT/dist/tools/rbpf/gen_rbf.py dump fc/increment.bin
Magic:          0x46504272
Version:        0
flags:  0x0
Data length:    0 B
RoData length:  16 B
Text length:    24 B
No. functions:  1

functions:
        "increment": 0x0

data:

rodata:
    0: 0x69 0x6e 0x63 0x72 0x65 0x6d 0x65 0x6e
    8: 0x74 0x00 0x00 0x00 0x00 0x00 0x00 0x00

text:
<increment>
    0x0:        79 10 00 00 00 00 00 00 r0 = *(uint64_t*)(r1 + 0)
    0x8:        07 00 00 00 01 00 00 00 r0 += 1
   0x10:        95 00 00 00 00 00 00 00 Return r0
```

Visible is the application header, the list of functions, the read only data
(only containing the function name and some padding) and the application code.
The application code fetches the value from the pointer in `r1` (the context
argument) and increments the value in the second instruction. By storing it in
register `r0` it is returned back to the operating system after the return
instruction.
