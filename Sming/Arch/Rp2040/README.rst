Sming RP2040 Architecture
=========================

.. highlight:: bash

Support building Sming for the `Raspberry Pi RP2040 SOC
<https://www.raspberrypi.org/documentation/microcontrollers/raspberry-pi-pico.html>`__.

Testing so far has been limited to the Rasperry Pi Pico, but there are lots of other boards available.
Configure this using the :envvar:`PICO_BOARD` setting. The default is ``pico``.
You can find the `full list here <https://github.com/raspberrypi/pico-sdk/tree/master/src/boards/include/boards>`__.

Special mention to the arduino-pico project https://github.com/earlephilhower/arduino-pico.
Lots of helpful stuff in there!

.. note::

   This architecture should be considered experimental at present.

Tested and working:

- CPU frequency adjustment :cpp:func:`system_get_cpu_freq`, :cpp:func:`system_update_cpu_freq`
- Timers working: hardware, software and CPU cycle counter
- Hardware serial ports (UART driver)
- Task queue (also supports queuing tasks from code running on core #1)
- Flash memory routines
- :cpp:func:`os_random` and :cpp:func:`os_get_random` implemented using ring oscillator.
  This is the best the hardware is capable of, but not crypto grade.
- Heap is standard newlib implementation, :cpp:func:`system_get_free_heap_size` provided.
- Software watchdog implemented, timeout is 8 seconds
- A disassembly and symbol file are generated for this architecture.
- SDK declares flash memory size in the board header files.
  This is checked at compile time against the value declared in the partition table.
- Reset information :cpp:func:`system_get_rst_info` indicates watchdog or manual resets.
  Exception information not yet implemented.
- System functions :cpp:func:`system_get_chip_id`, :cpp:func:`system_get_sdk_version`.
- Partitions and file systems (except SD cards and FAT)
- SPIClass tested with Radio_nRF24L01 sample only
- WiFi networking support for the Pico-W
- Standard analogue I/O via analogRead. More advanced hardware capabilities require use of the SDK directly.
- Dual-core support. See below for details.
- USB supported using the :library:`USB` library, both host and device modes.
- HardwareSPI via :library:`HardwareSPI` for fully asynchronous SPI communications (host mode only).

Yet to be implemented:

PWM
   Hardware can drive up to 16 outputs and measure input frequency/duty cycle.
   The native API is quite straightforward to use and makes best use of the hardware.
I2C
   Has hardware support
RTC
   Can wake from deep sleep but requires an external clock (e.g. 32kHz crystal) and appropriate API.
   (Setting and reading the time is implemented.)
Low-power modes
   Deep sleep / suspend / power-saving
PIO (Programmable I/O)
   A killer feature for the RP2040.
   Uses range from simple glue logic to I2S, etc.
Crash/exception handling & serial debugging
   RP2040 supports JTAG debugging but requires extra hardware.
   Serial debugging is often enough and easier to set up.
   Requires GDB stub plus implementing crash handler callbacks, etc.
Multi-boot / OTA updates.
   If you run ``make map`` you'll see there is no bootloader!
   It's part of the firmware image at present.
   Adding RP2040 support to rBoot may work, however the Pico typically has only 2MByte flash which is quite restrictive.
   It is also necessary to compile images at different addresses as there is no windowed XIP (eXecute In Place) capability.
   See :library:`FlashIP` library for a basic method of OTA.


Requirements
------------

These requirements are in addition to the standard Sming setup.

The easiest way to get started is with the Sming installer - see :doc:`/getting-started/index`.
For Linux and MacOS just run ``Tools/install.sh rp2040``.

Note: Windows is not currently included in the chocolatey repository.
The following instructions should help.

Compiler/linker
   The RP2040 contains two ARM Cortex-M0+ cores. Tools for all platforms can be downloaded from the
   `ARM developer website <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>`__.

   Unzip the archive to a suitable location (e.g. ``/opt/rp2040`` or ``c:\tools\rp2040``) and set :envvar:`PICO_TOOLCHAIN_PATH` accordingly.

   You can alternatively use the toolchains provided in your GNU/Linux distribution.

   Ubuntu

      ::

         sudo apt install gcc-arm-none-eabi gdb-multiarch

      To use gdb-multiarch you'll need to do this::

         make gdb GDB=gdb-multiarch

   Fedora

      ::

         sudo dnf install arm-none-eabi-gcc-cs-c++ arm-none-eabi-newlib

      The standard GDB appears to work OK.

Ninja

   This is used to build the RP2040 SDK code::

      sudo apt install ninja-build

   It is available for other platforms at https://ninja-build.org/
   and consists of a single executable file.
   The application should either be in the system path, or set :envvar:`NINJA`
   to the full path of the executable file.

   If you have Sming working with the ESP32 then you'll already have it installed.


Setup and programming
---------------------

Serial support requires a 3.3v USB adapter connected to the appropriate pins:

- UART0: TX = 0, RX = 1
- UART1: TX = 4, RX = 5

To program your device, unplug the USB cable (i.e. remove power from the device)
then hold down the ``BOOTSEL`` button whilst plugging it back in again.

You can then run:

   make flash

as usual and the device will be programmed.

Once Sming is running on the device, reprogramming is simpler and only requires pressing
the ``BOOTSEL`` button (no power cycle).

If the firmware has crashed or stalled the watchdog timer should reboot the system after 8 seconds,
at which point BOOTSEL should be detected. So just hold the button down until this happens.

If all else fails, go through the initial power-cycle process.

This behaviour can be disabled using the :envvar:`ENABLE_BOOTSEL` setting.


Boot process
------------

Unlike the Espressif parts, the RP2040 is not programmed via the serial port,
but written to the device when configured as a Mass Storage device (removable flash drive).

Data to be flashed must be in `UF2 format <https://github.com/Microsoft/uf2>`__ and
sent as a single file. See :component-rp2040:`uf2`.

Once the file has finished sending the RP2040 reboots itself into normal operating mode
(assuming BOOTSEL has been released).

The RP2040 can also be programmed via JTAG debugging but this requires additional hardware and setup.

.. note::

   The RP2040 bootloader does not include support for reading flash memory via mass storage,
   so commands such as ``make verifyflash`` won't work at present.


Dual-core support
-----------------

Sming is a strictly non-threaded framework, and all code runs on core #0.
The SDK *multicore* API may still be used to run code on core #1, but this requires some care to ensure smooth operation.

The task queue (:cpp:func:`System::queueTask`, etc.) may be used to send messages to Sming from Core #1 code.

Passing messages the other way, from Sming code to core #1, could be done using a separate SDK task queue.


Flash access
~~~~~~~~~~~~

Core 1 code may run directly from flash memory (via XIP) without any special considerations.
However, during flash erase/write operations (e.g. file writes) XIP is disabled.
If core 1 code attempts to access flash during these periods the system will hard fault.

.. note::

   Floating-point support requires use of routines in flash memory.
   Integer operations should all be safe to use.

   If unexplained crashes are occurring then check the build output files (in out/Rp2040/debug/build)
   or use a debugger to identify any errant code running from flash.

A typical use for core #1 might be to perform processing of some kind, such as processing data sampled
via analogue inputs. If all code is run from RAM then it can continue uninterrupted even during filing system
operations.

Alternatively some kind of synchronisation mechanism may be used to ensure that core 1 is suspended or running from RAM
during any flash erase/write operations.



Networking
----------

The Pico-W variant includes an Infineon CYW43439 bluetooth/WiFi SoC.

Raspberry Pi use the ... driver. The SDK also includes an LWIP implementation.

The physical interface is SPI using a custom (PIO) implementation.
This requires the use of GPIOxx which can no longer be accessed directly,
but instead via xxxxx.

The CYW43 chip is initialised (via `cyw43_ensure_up`) when application code
makes the first call into the networking API, for example by enabling station
or AP access. Part of the hardware configuration here is to download firmware
to the CYW43 chip (about 240KB) plus the CLM BLOB (< 1KB).

.. note:

   CLM stands for ``Country Locale Matrix``. The data defines regulatory configuration (target power outputs).
   Currently a 'global' setting is used to initialise WiFi, but there may be advantages in changing this to the
   specific country where the device is being deployed.

Sming contains patches which compresses this data (based on https://github.com/raspberrypi/pico-sdk/issues/909)
to about 145KB.
By default, it is linked into the application image, but can also be read from a separate partition.


Source code
-----------

The RP2040 is a very capable SOC, albeit without WiFi.
A massive advantage is that the platform is fully open-source.
Even the bootrom is published!

Here's a summary of the various Github repositories the Raspberry Pi Foundation have made available:

https://github.com/raspberrypi/pico-sdk
   The core SDK for the RP2040 SOC. Sming includes this as a submodule.

https://github.com/raspberrypi/picotool
   This is a tool for inspecting RP2040 binaries, and interacting with RP2040 devices
   when they are in BOOTSEL mode.
   It does this by talking to a custom USB device implemented in the RP2040 bootrom.

   Getting this to build is a bit fiddly.
   So far I've managed without it, but there is a ``picotool`` component in the framework
   which can be used to try building it.

https://github.com/raspberrypi/pico-bootrom
   Contents of the RP2040 boot rom. Very handy to be able to see this.

https://github.com/raspberrypi/pico-examples
   Examples using the pico SDK directly.

https://github.com/raspberrypi/picoprobe
   An RP2040 board can be used as a low-cost JTAG adapter using this firmware.
   Takes some setting up to use though.
   See [Getting Started PDF](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
   for details.

https://github.com/raspberrypi/pico-extras
   Some additional libraries which may or may not end up in the SDK.

https://github.com/raspberrypi/pico-playground
   Further examples using the pico-extras libraries.


Configuration Variables
-----------------------

.. envvar:: PICO_TOOLCHAIN_PATH

   This contains the base directory for the toolchain used to build the framework.
   Pre-compiled toolchains can be downloaded from the
   `ARM Developer website <https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads>`__.


Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index
