WSL
===

.. highlight:: batch

Building under Windows is generally slower than in Linux.
This is because the current build system requires a Posix emulation layer (MinGW).
However, it does offer the simplest way to use Sming on a Windows PC and does not affect the quality
or functionality of your applications.

There are situations where it is highly desirable to build Sming in a Linux environment:

-  Making use of linux-only development tools, e.g. valgrind (dynamic bug detection system)
-  Integration building/testing prior to submitting a PR to the Sming repository
-  Need/want faster builds

Whilst a Windows computer can be configured to dual-boot with Linux, this is generally inconvenient
for day-to-day use. A better solution is to run Linux inside a virtual machine environment such as
`VirtualBox <https://www.virtualbox.org/>`__,
`VmWare <https://www.vmware.com/>`__ or
`Hyper-V <https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/about/>`__.

Note that `Docker <https://www.docker.com/resources/what-container>`__ is not a virtual
environment but can in fact be run inside a virtual machine to take advantage of the
process isolation and security benefits.


Windows Subsystem for Linux
---------------------------

https://docs.microsoft.com/en-us/windows/wsl/

"The Windows Subsystem for Linux lets developers run a GNU/Linux environment
-- including most command-line tools, utilities, and applications --
directly on Windows, unmodified, without the overhead of a traditional virtual machine or dual-boot setup."

There are currently two versions of WSL: this documentation relates to **WSL2**.

.. note::

   WSL2 uses Hyper-V so may conflict with other virtual machines you may be using.


Installing WSL
--------------

See instructions here https://docs.microsoft.com/en-us/windows/wsl/install.

Install an up-to-date Linux distribution from the Microsoft store, currently ``Ubuntu-20.04``.

.. note::

   You may encounter an error message similar to this during installation::

      WslRegisterDistribution failed with error: 0x80370102
      Error: 0x80370102 The virtual machine could not be started because a required feature is not installed.

   One thing not mentioned in the instructions is to check that the hypervisor is set to auto-start at system boot.
   This is the default but for various reasons it can get disabled.

   To check, type::
   
      bcdedit

   At an administrative command prompt. Under the ``Windows Boot Loader`` entry you should see an entry like this::

      hypervisorlaunchtype    Auto

   If it's missing or set to another value (e.g. ``off``) then change it as follows::

      bcdedit /set {current} hypervisorlaunchtype auto

   After a system reboot you should be able to continue with the installation.



Installing Sming
----------------

Open a WSL command prompt and follow the instructions in :doc:`../linux/index`.

Please note:

- A native Windows python3 distribution is required to enable access to serial ports.
- Ensure that python is available in the system path for both WSL2 and Windows.
- Do not set the :envvar:`PYTHON` environment variable.

This will ensure that the build system can run python scripts either in WSL2 or in Windows as necessary.


Flashing devices
----------------

WSL2 does not natively support access to USB serial devices, so the Sming build system runs
the appropriate application directly under Windows using ``powershell.exe``.

Therefore, use the normal Windows COM port name rather than the linux ones (such as /dev/ttyUSB0)::

   make flash COM_PORT=COM4


Some USB serial adapters are supported by the `usbipd <https://github.com/dorssel/usbipd-win>`__ project.
If so, then devices such as ``/dev/ttyUSB0``, etc. will be present as usual.
If Sming sees that ``COM_PORT`` actually exists when running in WSL2 then the above powershell workaround
will not be used.

.. note::
   
   The :envvar:`TERMINAL` environment variable is cached by Sming so if the terminal isn't running as expected
   try ``make config-clean`` to clear the cached value.


Serial debugging
----------------

Again, as we have no direct access to USB COM ports a workaround is required.
A small python application can be run on Windows to act as a simple bridge between the serial port and a TCP port.
See ``Tools/tcp_serial_redirect.py`` - run without arguments to see available options.

You can start the server like this::

   make tcp-serial-redirect

A new console will be created (minimised) showing something like this::

   --- TCP/IP to Serial redirect on COM4  115200,8,N,1 ---
   --- type Ctrl-C / BREAK to quit
   Waiting for connection on 192.168.1.101:7780...

This uses the current :envvar:`COM_PORT` and :envvar:`COM_SPEED_SERIAL` settings.

Now we can start the debugger::

   make gdb COM_PORT_GDB=192.168.1.101:7780


Valgrind
--------

You may get an error running ``make valgrind`` advising that ``libc6-dbg:i386`` be installed. Here's how::

   sudo dpkg --add-architecture i386
   sudo apt-get update
   sudo apt-get install libc6-dbg:i386

