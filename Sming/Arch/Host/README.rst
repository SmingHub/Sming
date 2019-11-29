Sming Host Emulator
===================

.. highlight:: bash

Summary
-------

Allows Sming applications to be built as an executable to run on the
development host (Windows or Linux).

This is a source-level emulator for developing and testing new framework
code prior to flashing a real device.

This is not a machine emulator; if you need something operating at a
lower level take a look at `QEMU <https://www.qemu.org/>`__.

Requirements
------------

``CMake`` is required to build LWIP

Ensure you are using relatively recent compilers, with 32-bit libraries available.

For Linux, you may require the ``gcc-multilib`` and ``g++-multilib``
packages to build 32-bit executables on a 64-bit OS.

For Windows, make sure your ``MinGW`` distro is up to date.
See :doc:`/arch/esp8266/getting-started/windows-manual` for further details.

Building
--------

Build the framework and application as usual, specifying :envvar:`SMING_ARCH` =Host. For example::

   cd $SMING_HOME
   make SMING_ARCH=Host
   cd $SMING_HOME/../samples/Basic_Serial
   make SMING_ARCH=Host

This builds the application as an executable in, for example,
``out/Host/firmware/app.exe``. Various command-line options are
supported, use ``--help`` for details.

The easiest way to run the emulator is via ``make run``. Variables are
used to pass the appropriate options to the command line.

To find out what options are in force, use ``make list-config``.


Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index


todo
----

* Add passthrough support for real serial ports to permit connection of physical devices.
* Consider how this mechanism might be used to support emulation of other devices (SPI, I2C, etc).
* Development platforms with SPI or I2C (e.g. Raspberry Pi) could be supported.
* Are there any generic device emulators available? For example, to simulate specific types of SPI slave.
