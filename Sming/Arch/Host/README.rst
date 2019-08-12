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

For Windows, make sure your ``MinGW`` distro is up to date. If you run
``gcc --version`` you should get ``gcc (MinGW.org GCC-6.3.0-1) 6.3.0``
or later. If it’s older, execute these commands:

.. code-block:: powershell

   mingw-get update
   mingw-get upgrade

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
used to pass the appropriate options (see `features`_).

To find out what options are in force, use ``make list-config``.

.. _features:

Features
--------

Flash memory
~~~~~~~~~~~~

This is emulated using a backing file. By default it’s in ``flash.bin``
in the firmware directory, you can change it by setting :envvar:`FLASH_BIN`.
The size of the flash memory is set via :envvar:`SPI_SIZE`.

-  ``make flashinit`` to clear and reset the file.
-  ``make flashfs`` to copy the generated SPIFFS image into the backing file.
-  ``make flash`` writes out all required images to the backing file.
   For now, this is the same as ``make flashfs`` but that will change
   when support is added for custom user images.

UART (serial) ports
~~~~~~~~~~~~~~~~~~~

Multiple serial terminals are supported via raw TCP network sockets, so
telnet can be used to provide terminal capability.

``make run`` starts the emulator with any required telnet sessions. By
default, no serial ports are enabled, however any output from UART0 is
redirected to the console. No input is possible.

If your project requires proper terminal access, set :envvar:`ENABLE_HOST_UARTID`.

Set :envvar:`HOST_UART_PORTBASE` if you want to change the base port number
used to communicate with the emulator.

Alternatively, you can run the application manually like this:

``out/firmware/app --pause --uart=0 --uart=1``

Now start a telnet session for each serial port, in separate command windows::

   telnet localhost 10000
   telnet localhost 10001

In the application window, press Enter. This behaviour is enabled by the
``pause`` option, which stops the emulator after initialisation so
telnet can connect to it. Without ``pause`` you’ll lose any serial
output at startup.)

Note: For Windows users, ``putty`` is a good alternative to telnet. It also
has options for things like carriage-return/linefeed translation
(“\\n” -> “\\r\\n`”). Run using::

   putty telnet://localhost:10000

Port numbers are allocated sequentially from 10000. If you want to use
different port numbers, use ``--uartport`` option.

Digital I/O
~~~~~~~~~~~

By default, the emulator just writes output to the console so you can see when outputs are changed.

Reading from an input returns 0.

All digital functions can be customised by overriding the *DigitalHooks* class, like this:

.. code-block:: c++

   // You'd probably put this in a separate module and conditionally include it
   #ifdef ARCH_HOST
   class MyDigitalHooks: public DigitalHooks
   {
   public:
      // Override class methods as required
      uint8_t digitalRead(uint16_t pin, uint8_t mode) override
      {
         if(pin == 0) {
            return 255;
         } else {
            return DigitalHooks::digitalRead(pin, mode);
         }
      }
   };
   
   MyDigitalHooks myDigitalHooks;
   #endif

   void init()
   {
      #ifdef ARCH_HOST
      setDigitalHooks(&myDigitalHooks);
      #endif
   }

See :source:`Sming/Arch/Host/Core/DigitalHooks.h` for further details.


Network
~~~~~~~

Linux
^^^^^

Support is provided via TAP network interface (a virtual network layer
operating at the ethernet frame level). A TAP interface must be created
first, and requires root priviledge::

   sudo ip tuntap add dev tap0 mode tap user `whoami`
   sudo ip a a dev tap0 192.168.13.1/24
   sudo ifconfig tap0 up

This creates the ``tap0`` interface. The emulator will automatically
select the first ``tap`` interface found. To override this, use the
``--ifname`` option. An IP address will be assigned, but can be changed
using the ``--ipaddr`` option.

If your application needs to access the internet, additional setup is
required::

   sudo sysctl net.ipv4.ip_forward=1
   sudo sysctl net.ipv6.conf.default.forwarding=1
   sudo sysctl net.ipv6.conf.all.forwarding=1

   export INTERNET_IF=wlan0 # <!--- Make sure to replace wlan0 with the network interface connected to Internet

   sudo iptables -t nat -A POSTROUTING -o $INTERNET_IF -j MASQUERADE
   sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
   sudo iptables -A FORWARD -i tap0 -o $INTERNET_IF -j ACCEPT

Windows
^^^^^^^

Requires `NPCAP <https://nmap.org/npcap/>`__ library to be installed.
Provided with current (3.0.2) version of
`Wireshark <https://www.wireshark.org/download.html>`__.

By default, the first valid network adapter will be used, with address
assigned via DHCP.

If the adapter is wrong, get a list thus:

.. code-block:: batch

   out\Host\Windows\debug\firmware\app --ifname=?

or

.. code-block:: batch

   make run HOST_NETWORK_OPTIONS=--ifname=?

produces a listing:

.. code-block:: text

   Available adapters:
   - 0: {ACC6BFB2-A15B-4CF8-B93A-8D97644D0AAC} - Oracle
           192.168.56.1 / 255.255.255.0
   - 1: {A12D4DD0-0EA8-435D-985E-A1F96F781EF0} - NdisWan Adapter
   - 2: {3D66A354-39DD-4C6A-B9C4-14EE223FC3D1} - MS NDIS 6.0 LoopBack Driver
           0.0.0.0 / 255.0.0.0
   - 3: {BC53D919-339E-4D70-8573-9D7A8AE303C7} - NdisWan Adapter
   - 4: {3CFD43EA-9CC7-44A7-83D4-EB04DD029FE7} - NdisWan Adapter
   - 5: {530640FF-A9C3-436B-9EA2-65102C788119} - Realtek PCIe GBE Family Controller
           192.168.1.70 / 255.255.255.0
   - 6: {0F649280-BAC2-4515-9CE3-F7DFBB6A1BF8} - Kaspersky Security Data Escort Adapter
           10.102.37.150 / 255.255.255.252

Then use the appropriate number (or GUID), with the gateway IP address -
an address will be assigned via DHCP:

.. code-block:: batch

   make run HOST_NETWORK_OPTIONS="--ifname=5 --gateway=192.168.1.254"

You can find gateway addresses using the ``ipconfig`` command.

If you want to use a specific IP address, the appropriate adapter will
be selected but you still need to specify the gateway address:

.. code-block:: batch

   make run HOST_NETWORK_OPTIONS="--ipaddr=192.168.1.10 --gateway=192.168.1.254"


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
