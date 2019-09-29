Memory
======

Introduction
------------

You can find a map for the ESP8266 memory layout in the `Wiki <https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map>`__.

The ESP8266 has several types of memory, and it is important to have a basic apprecation of what they
are and how they're used.

DRAM
   Data RAM where variables, etc. are stored. Contains the stack (which starts at the top of RAM) 
   and the heap (which starts near the bottom, after any static data).

IRAM
   Instruction RAM. All code executes from here, but there's not much of it so
   so only parts of your application are loaded at any one time. This *caching* happens
   in the background by mapping certain memory address ranges into physical flash
   locations.

   If a location is accessed which is already in the cache (a 'hit') then the access
   runs at full RAM speed. If it's not in the cache (a 'miss') then an interrupt (exception)
   is generated internally which performs the flash read operation.

   This is why interrupt service routines must not access PROGMEM directly, and must
   be marked using *IRAM_ATTR* to ensure it's *always* available.

   You *may* get a performance improvement using *IRAM_ATTR* but  
   means that commonly-used code does not necessarily run faster if explicitly
   stored in IRAM.


Flash
   Main storage for your application, libraries, the Espressif SDK code, etc.
   Flash memory is accessed via external serial bus and is relatively slow.
   For the ESP8266, it's approximately 12x slower, though this only applies
   on cache misses. See also :doc:`flash`.

ROM
   Fixed code stored in the Esp8266 itself containing very low-level support code which
   is factory-programmed and cannot be changed.


Initialisation
--------------

At startup, only the non-volatile *Flash* and *ROM* memories contain anything useful,
whilst *DRAM* and *IRAM* will probably just contain garbage. The Arduino platform
was initially designed to work with much simpler hardware, where the program was executed
directly from Flash memory on hardware reset.

BIOS
   The ESP8266 and ESP32 are far more complex, and most of the low-level initialisation
   happens in the ROM code. The ROM essentially contains the systems BIOS, with various
   low-level routines which may be used instead of accessing hardware directly. It is
   also responsible for setting up memory caching.

Runtime libraries
   Control is passed to runtime libraries provided by Espressif, stored in Flash memory.
   Both ROM and runtime code are closed-source and not generally available for inspection,
   though disassemblies do exist.

Boot loader
   The first point we really see what's going on is in the bootloader (rBoot).
   The bootloader identifies the correct program image (as there can be more than one),
   loads the starting portion into IRAM and jumps there. It also configures the caching
   mechanism so that the correct program image is  loaded.
   You can find more details about this in the :component:`rboot` documentation.

Memory initialisation
   Code is copied from flash into IRAM, and *const* data copied into DRAM.
   Also static and global variable values are initialised from tables stored in flash.
   Static and global variables without an initialised value are initialised to 0.

Sming initialisation
   ...
