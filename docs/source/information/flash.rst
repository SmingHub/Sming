Flash memory
============

.. highlight:: C++

Introduction
------------

Size varies from 512Kbytes on the ESP-01 up to 4Mbytes on the ESP12F. Up to 16MBytes are supported for custom designs.

You can find general details for the ESP8266 memory layout in the `Wiki <https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map>`__.

This is the layout for Sming with a 4MByte flash device:

=======  ===============   ====   =========================  ===================================================                                
Address  Config variable   Size   Source filename            Description            
(hex)    (if any)          (KB)   (if applicable)            
=======  ===============   ====   =========================  ===================================================            
000000                     1      rboot.bin                  Boot loader            
001000                     4                                 rBoot configuration            
002000   ROM_0_ADDR               rom0.bin                   First ROM image            
100000   RBOOT_SPIFFS_0
202000   ROM_1_ADDR               rom1.bin                   Second ROM image            
300000   RBOOT_SPIFFS_1
3FB000                     4      blank.bin                  RF Calibration data (Initialised to FFh)
3FC000                     4      esp_init_data_default.bin  PHY configuration data            
3FD000                     12     blank.bin                  System parameter area
=======  ===============   ====   =========================  ===================================================            


Partition Tables
----------------

.. todo::

   Whilst SDK version 3 requires a partition table, previous versions do not but this can be added so that we
   can use it as a common reference for all the above locations.


Flash Memory
------------

Speed and caching
~~~~~~~~~~~~~~~~~

Flash memory on the ESP8266 is accessed via an external SPI bus, so reading it takes about 12x
longer than reading from internal RAM. To mitigate this, some of the internal RAM is used to
cache data. Part of this is managed in hardware, which means if the data required is already in
the cache then there is no difference in speed. In general, then, frequently accessed data is read
as if it were already in RAM.

Bear in mind that every time new data is read via the cache, something else will get thrown away
and have to be re-read. Therefore, if you have large blocks of infrequently accessed data then
it's a good idea to read it directly using *flashmem_read()*. You can get the address for a
memory location using *flashmem_get_address()*.

PROGMEM
~~~~~~~

A string literal (e.g. "string") used in code gets emitted to the *.rodata* segment by the compiler.
That means it gets read into RAM at startup and remains there.

To avoid this, and reclaim the RAM, the data must be stored in a different segment. This is done
using the ``PROGMEM`` macro, defined in :source:`Sming/Wiring/FakePgmSpace.h`.

memcpy_aligned
~~~~~~~~~~~~~~

Once in flash memory, string data must be read into RAM before it can be used. Accessing the flash
memory directly is awkard. If locations are not strictly accessed as 4-byte words the system will
probably crash; I say 'probably' because sometimes it just behaves weirdly if the RAM address
isn't aligned.

So, the location being accessed, the RAM buffer it's being copied to and the length all have to be
word-aligned, i.e. integer multiples of 4 bytes.
If these conditions are satisfied, then it's safe to use a regular *memcpy* call.
However, you are strongly discouraged from doing this.
Instead, use *memcpy_aligned* which will check the parameters and raise an assertion in debug mode
if they are incorrect.

FakePgmSpace
~~~~~~~~~~~~

:source:`Sming/Wiring/FakePgmSpace.h` also provides the basic mechanisms for storing and reading flash strings,
including versions of standard string library functions such as *memcpy_P*, *strcpy_P*, etc.
These are well-documented Arduino-compatible routines.

Some additions have been made to Sming to cater for the ESP8266 use of these strings.

F(string_literal)
   Loads a String object with the given text, which is allocated to flash::
   
      String s = F("test");

   .. note::
   
      The ``F()`` macro differs slightly from the Arduino/Esp8266 implementation in that it instantiates a ``String`` object.

      Since the length of the string is known at compile-time, it can be passed to the String
      constructor which avoids an additional call to *strlen_P()*.


_F(string_literal)
   Like F() except buffer is allocated on stack. Most useful where nul-terminated data is required::

      m_printf(_F("C-style string\n"));

DEFINE_PSTR(name, str)
   Declares a PSTR stored in flash. The variable (name) points to flash memory so must be accessed
   using the appropriate xxx_P function.

LOAD_PSTR(name, flash_str)
   Loads pre-defined PSTR into buffer on stack::

      // The ``_LOCAL`` macro variants include static allocation
      DEFINE_PSTR_LOCAL(testFlash, "This is a test string\n");
         LOAD_PSTR(test, testFlash)
         m_printf(test);

PSTR_ARRAY(name, str)
   Create and load a string into the named stack buffer.
   Unlike _F(), this ensures a loaded string stays in scope::
   
      String testfunc() {
         PSTR_ARRAY(test, "This is the test string");
         m_printf(test);
         ...
         return test;
        }

Both DEFINE_PSTR and PSTR_ARRAY load a PSTR into a stack buffer, but using sizeof() on that buffer will return
a larger value than the string itself because it's aligned. Calling sizeof() on the original flash data will
get the right value. If it's a regular nul-terminated string then strlen_P() will get the length, although it's
time-consuming.

FlashString
~~~~~~~~~~~

For efficient, fast and flexible use of PROGMEM data see :component:`FlashString`.
