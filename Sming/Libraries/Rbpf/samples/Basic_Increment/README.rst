Basic_Increment 
===============

.. highlight:: bash

Introduction
------------

This sample is a starting point for working with the rBPF system on Sming. 
It shows what is required as toolchain and how to get started with
loadable applications based on rBPF and Femto-Containers.

Steps
-----

Toolchain setup
~~~~~~~~~~~~~~~

This step describes the additional requements needed to compile
and execute the sample presented here. The following applications and
libraries are required:

	- LLVM
	- Clang
	- `pyelftools <https://github.com/eliben/pyelftools>`_

On Ubuntu the following two commands will install the needed toolchain and python dependencies::

	sudo apt-get install llvm clang
	cd path/to/this/folder
	make python-requirements
 
Please ensure that llvm supports generating eBPF code by executing::

	llc --version

This must show "bpf" in the list of registered targets.

Compiling the Femto-Container application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The code itself, as visible in the `container/increment.c` source file is only a single
functions. It receives the argument and interprets it as a pointer to a 64 bit
number. It increments the number and returns it.

.. code:: c++

	int increment(unsigned long* context)
	{
		return *context + 1;
	}

The source code of the Femto-Container application is in the `container` subdirectory. 
Compiling the code is done with make::

	make blobs

This uses Clang and LLVM to compile the source code to an eBPF object file which
is then converted via a python application to a Femto-Container-specific format.
The resulting binary has the same name as the source file, but with `.bin` as
extension. In this case the `increment.c` file is compiled to `increment.o` and
converted to `increment.bin`. The `.o` file can be inspected using the binutils
tools such as objdump. The content of the Femto-Container bin file can be
inspected using the tooling described further in this document.

Compiling the sample
~~~~~~~~~~~~~~~~~~~~

Once you have compiled increment.c you can compile this sample.
This sample supports the ``Host`` architecture which means that 
you can compile it and run it as a regular application using the following 
commands::

	make SMING_ARCH=Host
	make flash run


And should show output similar to::

	All up, running the Femto-Container application now
	Input to the VM: 0
	Return code (expected 0): 0
	Result of the VM: 1


As visible, the virtual machine application receives `0` as input value and
returns `1` back to the operating system. The return code of the virtual machine
is `0`, indicating a successful execution of the application.

Inspecting the Femto-Container application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As mentioned before, the Femto-Container application binary can be inspected
with tools. For example we can show the content of the container application 
using the following command::

	make blobs-dump
	
This will display a content similar to the one below::

	Magic:		0x46504272
	Version:	0
	flags:	0x0
	Data length:	0 B
	RoData length:	16 B
	Text length:	24 B
	No. functions:	1
	
	functions:
		"increment": 0x0
	
	data:
	
	rodata:
	    0: 0x69 0x6e 0x63 0x72 0x65 0x6d 0x65 0x6e
	    8: 0x74 0x00 0x00 0x00 0x00 0x00 0x00 0x00
	
	text:
	<increment>
	    0x0:	79 10 00 00 00 00 00 00 r0 = *(uint64_t*)(r1 + 0)
	    0x8:	07 00 00 00 01 00 00 00 r0 += 1
	   0x10:	95 00 00 00 00 00 00 00 Return r0


Visible is the application header, the list of functions, the read only data
(only containing the function name and some padding) and the application code.
The application code fetches the value from the pointer in `r1` (the context
argument) and increments the value in the second instruction. By storing it in
register `r0` it is returned back to the operating system after the return
instruction.
