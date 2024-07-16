Sming Host Emulator
===================

.. highlight:: bash

Sming allows most libraries and sample applications to be compiled on a Linux/MacOs/Windows
development system and be tested before uploading them to the microcontroller.

If you want to try it we have an
`interactive tutorial <https://killercoda.com/slaff/scenario/sming-host-emulator>`__
that can be run directly from your browser.

For installation details see :doc:`/getting-started/index`.

Summary
-------

This is a source-level emulator for developing and testing new framework
code prior to flashing a real device.

This is not a machine emulator; if you need something operating at a
lower level take a look at `QEMU <https://www.qemu.org/>`__.

The design goals for Host builds are:

- Simplify development of complex applications
- Enable use of advanced testing and code quality tools (valgrind, code sanitizers)
- Support CI testing by actually executing code, not just building it


Requirements
------------

``CMake`` and ``Ninja`` are required to build some libraries (e.g. lwip).

Tested compilers:

- GCC version 8 or later
- Clang version 15 or later
- Apple-clang version 14 or later

.. _linux_requirements:

Linux
~~~~~

Host executables can be built in 32-bit emulation mode, which is the default for Linux.
This requires installation of 32-bit runtime libraries.
For debian/ubuntu::

   sudo apt install g++-multilib

For Fedora::

   sudo dnf install glibc-devel.i686 libstdc++.i686

Alternatively set :envvar:`BUILD64` to 1 to build in native 64-bit mode.

MacOS
~~~~~

MacOS comes pre-installed with ``Apple Clang`` as the standard toolchain.
This should be sufficient to build Sming in Host mode.
Note that MacOS does not support 32-bit applications so the emulator will build in 64-bit mode.

Windows
~~~~~~~

For Windows, make sure your ``MinGW`` distro is up to date.
See :doc:`/getting-started/windows/index` for further details.


Building
--------

Environment variables
~~~~~~~~~~~~~~~~~~~~~

:envvar:`SMING_ARCH` must be set to use ``Host`` as the desired architecture::

   export SMING_ARCH=Host

Debug Build
~~~~~~~~~~~

If you plan to use a debugger make sure to set :envvar:`ENABLE_GDB` and (optionally)
:envvar:`ENABLE_LWIPDEBUG` before compiling the code::

   export ENABLE_GDB=1
   export ENABLE_LWIPDEBUG=1  # <!-- this will compile also LWIP with debug symbols

Initial Clean-up
~~~~~~~~~~~~~~~~

Make sure that you are working on a clean source code tree. The following commands may help::

   cd $SMING_HOME
   make dist-clean

Sample compilation
~~~~~~~~~~~~~~~~~~

Compile a sample as usual::

   cd $SMING_HOME/../samples/Basic_Blink
   make

The easiest way to run the emulator is via ``make run``. Variables are
used to pass the appropriate options to the command line.

To find out what options are in force, use ``make list-config``.

Note: The application is built as an executable in, for example, ``out/Host/firmware/app``,
which may be run directly. Various command-line options are supported, use ``--help`` for details.

Adapting existing code
~~~~~~~~~~~~~~~~~~~~~~

You may need to modify your existing applications to work with the emulator.

Architecture-specific code should be kept in separate code modules.
These can be build and linked as required using makefile logic.
For example, source code required only for Host builds can be placed in ``app/Host``
with these lines in the project's ``component.mk`` file::

   ifeq ($(SMING_ARCH),Host)
   COMPONENT_SRCDIRS += app/Host
   endif

There are plenty of examples of this in the framework and associated libraries,
such as :sample:`Basic_Tasks`.

Pre-processor macros are also defined for code use, such as :c:macro:`ARCH_ESP8266`.
So code that is specific to the ESP8266, for example, may be conditionally compiled using ``#ifdef ARCH_ESP8266``.
Similarly, you can check if the code is compiled for the emulator using ``#ifdef ARCH_HOST``.
See :envvar:`SMING_ARCH` and :envvar:`SMING_SOC`.


Running on the host
-------------------

Once the sample is compiled, initialise the :component-host:`vflash`::

   make flash

This writes all the prepared partitions (including any filing systems such as SPIFFS)
to the virtual flash backing file.

To run the application, do this::

   make run


Networking
~~~~~~~~~~

Support is provided via TAP network interface (a virtual network layer
operating at the ethernet frame level). A TAP interface must be created
first, and requires root privilege.

For Linux, you can use ``Sming/Arch/Host/Tools/setup-network-linux.sh``. Here is the manual approach::

   sudo ip tuntap add dev tap0 mode tap user `whoami`
   sudo ip a a dev tap0 192.168.13.1/24
   sudo ip link set tap0 up

   # The following lines are needed if you plan to access Internet
   sudo sysctl net.ipv4.ip_forward=1
   sudo sysctl net.ipv6.conf.default.forwarding=1
   sudo sysctl net.ipv6.conf.all.forwarding=1

   export INTERNET_IF=wlan0 # <!--- Make sure to replace wlan0 with the network interface connected to Internet

   sudo iptables -t nat -A POSTROUTING -o $INTERNET_IF -j MASQUERADE
   sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
   sudo iptables -A FORWARD -i tap0 -o $INTERNET_IF -j ACCEPT

This creates the ``tap0`` interface. The emulator will automatically
select the first ``tap`` interface found. To override this, use the
``--ifname`` option.

You can list available network interfaces thus::

   ip link


For MacOS, networking support first requires installation of a kernel extension
using ``Sming/Arch/Host/Tools/setup-network-macos.sh``.
On first run, you will need to confirm security then reboot the system.
On rebooting, run the script again.

The ``tap0`` network interface is created dynamically when ``/dev/tap0`` is opened.
At present, you must run the application with elevated permissions::

   sudo out/Host/debug/firmware/app



Troubleshooting
---------------

The following errors may be encountered when building:

- fatal error: sys/cdefs.h: No such file or directory
- fatal error: bits/c++config.h: No such file or directory

These indicate that the 32-bit support libraries are not installed.
See :ref:`Linux Requirements<linux_requirements>`.


Configuration
-------------

.. envvar:: CLI_TARGET_OPTIONS

   Use this to add any custom options to the emulator command line. e.g.:

      make run CLI_TARGET_OPTIONS=--help
      make run CLI_TARGET_OPTIONS="--debug=0 --cpulimit=2"

   Note: These settings are not 'sticky'


.. envvar:: CLANG_BUILD

   0: Use GCC (default)
   1: Use standard ``clang``
   N: Use specific installed version, ``clang-N``


.. envvar:: BUILD64

   default: undefined

   Set to 1 to build in native 64-bit mode.
   On MacOS builds are 64-bit only. Default for other systems is 32-bit.


Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index
