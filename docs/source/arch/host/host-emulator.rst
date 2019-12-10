Getting Started: Linux/Windows
==============================

.. highlight:: bash

The latest versions of Sming allow most of components of the library and
the sample applications to be compiled on a Linux/Windows host system
and be tested before uploading them to the microcontroller.

If you want to try it we have an
`interactive tutorial <https://www.katacoda.com/slaff/scenarios/sming-host-emulator>`__
that can be run directly from your browser.

Requirements (Linux)
--------------------

Modern Linux distribution
~~~~~~~~~~~~~~~~~~~~~~~~~

Most popular modern distributions should be fine. At the moment we
develop only with Ubuntu 16.04 and 18.04.

C/C++ 32 bit compiler and libraries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If your OS is 64 bit then you should install also 32 bit C/C++ compiler and libraries.
For Ubuntu, install like this::

   sudo apt-get install gcc-multilib g++-multilib

You may wish to install the latest compilers for testing alongside the recent
:doc:`/arch/esp8266/getting-started/eqt` updates. Here's a summary of how to do this::

   sudo add-apt-repository ppa:ubuntu-toolchain-r/test
   sudo apt update
   sudo apt install gcc-9 g++-9
   
   # For 64-bit systems
   sudo apt install gcc-9-multilib g++-9-multilib

   # Set GCC 9 as default
   sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9

The current version is `GCC 9.2.1 <https://www.gnu.org/software/gcc/gcc-9>`__.

CMake 3.8 or newer
~~~~~~~~~~~~~~~~~~

For the compilation of LWIP ``CMake`` version 3.8 or newer is required.
In order to get newer CMake version under Ubuntu 16.04, do this::

   cd /tmp
   wget https://github.com/Kitware/CMake/releases/download/v3.14.4/cmake-3.14.4-Linux-x86_64.sh
   sudo mkdir /opt/cmake
   sudo sh cmake-3.14.4-Linux-x86_64.sh --prefix=/opt/cmake
   sudo mv /usr/bin/cmake /usr/bin/cmake.orig
   sudo ln -s /opt/cmake/bin/cmake /usr/bin/cmake

Requirements (Windows)
----------------------

For Windows, make sure your ``MinGW`` distro is up to date.
The current version is `GCC 8.2.0 <https://gcc.gnu.org/gcc-8>`__.

.. note::

   If you don't already have MinGW installed, see :doc:`/arch/esp8266/getting-started/windows` for the Esp8266.



Compilation
-----------

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

Adapting existing code
~~~~~~~~~~~~~~~~~~~~~~

You may need to modify your existing applications to work with the
emulator. Architecture-specific code should be moved into separate code
modules. Code that is specific to the ESP8266 may be conditionally
compiled using ``#ifdef ARCH_ESP8266``. Similarly, you can check if the
code is compiled for the emulator using ``#ifdef ARCH_HOST``.

Running on the host
-------------------

Once the sample is compiled, initialise the :component-host:`vflash`::

   make flash

If enabled, this will also create a SPIFFS file system and write it to flash.

To run the application, do this::

   make run

Networking
~~~~~~~~~~

Support is provided via TAP network interface (a virtual network layer
operating at the ethernet frame level). A TAP interface must be created
first, and requires root privilege. You can use the
``Sming/Arch/Host/Tools/setup-network-linux.sh``. Here is the manual approach::

   sudo ip tuntap add dev tap0 mode tap user `whoami`
   sudo ip a a dev tap0 192.168.13.1/24
   sudo ifconfig tap0 up

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

Troubleshooting
---------------

Issue 1: fatal error: sys/cdefs.h: No such file or directory
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure to install the 32bit version of the GNU C and C++ compiler,
development package and libraries.

Issue 2: fatal error: bits/c++config.h: No such file or directory
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure to install the 32bit version of the GNU C and C++ compiler,
development package and libraries.


Further reading
---------------

.. toctree::
   :titlesonly:

   /_inc/Sming/Arch/Host/README

