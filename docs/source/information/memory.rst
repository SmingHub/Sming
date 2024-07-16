Memory
======

This section refers to the ESP8266 but is applicable to all device architectures.

Map
---

You can find a map for the ESP8266 memory layout in the `Wiki <https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map>`__.


Memory Types
------------

There are several types of memory, and it is important to have a basic apprecation of what they
are and how they're used.

DRAM
~~~~

Data RAM where variables, etc. are stored. Contains the stack (which starts at the top of RAM) 
and the heap (which starts near the bottom, after any static data).

IRAM
~~~~

Instruction RAM. All code executes from here, but there's not much of it so
so only parts of your application are loaded at any one time. This *caching* happens
in the background by mapping certain memory address ranges into physical flash
locations.

If a location is accessed which is already in the cache (a 'hit') then the access
runs at full RAM speed. If it's not in the cache (a 'miss') then an interrupt (exception)
is generated internally which performs the flash read operation.

This is why interrupt service routines must not access PROGMEM directly, and must
be marked using :c:macro:`IRAM_ATTR` to ensure it's *always* available.

You *may* get a performance improvement using :c:macro:`IRAM_ATTR`, but as frequently-used
code will be cached anyway it won't necessarily run faster.

If the code is timing critical it may benefit from pre-caching.
See :component-esp8266:`spi_flash`.

Flash
~~~~~

Main storage for your application, libraries, the Espressif SDK code, etc.
Flash memory is accessed via external serial bus and is relatively slow.
For the ESP8266, it's approximately 12x slower, though this only applies
on cache misses.

See :doc:`flash` for further details.

ROM
~~~

Fixed code stored in the Esp8266 itself containing very low-level support code which
is factory-programmed and cannot be changed.


Initialisation
--------------

At startup, only the non-volatile *Flash* and *ROM* memories contain anything useful,
whilst *DRAM* and *IRAM* will probably just contain garbage. The Arduino platform
was initially designed to work with much simpler hardware, where the program was executed
directly from Flash memory on hardware reset.

BIOS
~~~~

Most of the low-level initialisation happens in the ROM code.
The ROM essentially contains the systems BIOS, with various
low-level routines which may be used instead of accessing hardware directly. It is
also responsible for setting up memory caching.

Runtime libraries
~~~~~~~~~~~~~~~~~

For Espressif devices, control is passed to the SDK runtime libraries, stored in Flash memory.
The ROM and some runtime code are closed-source and not generally available for inspection,
though disassemblies do exist.

Boot loader
~~~~~~~~~~~

The first point we really see what's going on is in the bootloader.

This is :component:`rboot` for the Esp8266; the Esp32 bootloader is part of the SDK.
The bootloader identifies the correct program image (as there can be more than one),

The Rp2040 bootloader is more basic as it only supports booting one image.

In all cases, the bootloader loads the starting portion of the program image into IRAM and jumps there.
It also configures the caching mechanism so that the correct program image is loaded.

Memory initialisation
~~~~~~~~~~~~~~~~~~~~~

This is done by startup code in the SDK before passing control to Sming.

Code is copied from flash into IRAM, and *const* data copied into DRAM.
Also static and global variable values are initialised from tables stored in flash.
Static and global variables without an initialised value are initialised to 0.

Sming initialisation
~~~~~~~~~~~~~~~~~~~~

This varies between architectures but involves the following general tasks:

Initialize hardware
   e.g. system clocks, timers, RTC

Invoke C++ initializers
   static/global constructors get called.
   We only need to do this for the Esp8266, the SDK handles it for other architectures.

Load partition table
   Reads partition information from flash into RAM.
   - ESP32: we get called from the SDK which does this independently
   - Esp8266: we need to tell SDK about various partitions
   - Rp2040: the SDK no concept of partition tables

Initialise networking
   If enabled.

Invoke the application's ``init`` function

Enter the main run loop
   Interaction with SDK to ensure that registered software timers and queued tasks
   are executed, and the watchdog timer serviced.


.. toctree::
   :maxdepth: 1

   flash
