Windows WSL
===========

.. highlight:: batch

Building under Windows is generally slower than in Linux.
This is because the current build system requires a Posix emulation layer (MinGW).
However, it does offer the simplest way to use Sming on a Windows PC and does not affect the quality
or functionality of your applications.

However, there are situations where it is highly desirable to build Sming in a Linux environment, such as:

-  Making use of linux-only development tools, such as valgrind (dynamic bug detection system)
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

There are currently two versions of WSL, dubbed WSL1 and WSL2.
Either is fine, and you can switch between versions but WSL2 is recommended. 

.. note::

   WSL2 uses Hyper-V so may conflict with other virtual machines you may be using.


Installing WSL
--------------

See instructions here https://docs.microsoft.com/en-us/windows/wsl/install-win10#manual-installation-steps.

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

Open a WSL command prompt and follow the instructions in :doc:`linux`.


Flashing devices
----------------

WSL2 does not currently support access to USB serial devices, so the Sming build system incorporates a workaround
which runs the appropriate application (esptool) directly under Windows (via powershell).

Therefore, use the normal Windows COM port name rather than the linux ones (such as /dev/ttyUSB0).

For example::

   make flash COM_PORT=COM4


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

