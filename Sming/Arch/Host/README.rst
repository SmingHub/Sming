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

MacOS comes pre-installed with ``Apple Clang`` as the standard toolchain.
This should be sufficient to build Sming in Host mode.

For Windows, make sure your ``MinGW`` distro is up to date.
See :doc:`/getting-started/windows/index` for further details.

Building
--------

Build the framework and application as usual, specifying :envvar:`SMING_ARCH` =Host. For example::

   cd $SMING_HOME/../samples/Basic_Serial
   make SMING_ARCH=Host

This builds the application as an executable in, for example,
``out/Host/firmware/app.exe``. Various command-line options are
supported, use ``--help`` for details.

The easiest way to run the emulator is via ``make run``. Variables are
used to pass the appropriate options to the command line.

To find out what options are in force, use ``make list-config``.


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

   Note: This setting is not 'sticky'


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
