Flash memory
============

.. highlight:: C++

Introduction
------------

ESP8266 flash memory sizes vary from 512Kbytes on the ESP-01 up to 4Mbytes on the ESP12F.
Up to 16MBytes are supported for custom designs.

You can find general details for the memory layout in the `ESP8266 Wiki <https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map>`__.

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

{ todo }

Whilst SDK version 3 requires a partition table, previous versions do not but this can be added so that we
can use it as a common reference for all the above locations.


Speed and caching
-----------------

Flash memory on the ESP8266 is accessed via an external SPI bus, so reading it takes about 12x
longer than reading from internal RAM. To mitigate this, some of the internal RAM is used to
cache data. Part of this is managed in hardware, which means if the data required is already in
the cache then there is no difference in speed. In general, then, frequently accessed data is read
as if it were already in RAM.

Bear in mind that every time new data is read via the cache, something else will get thrown away
and have to be re-read. Therefore, if you have large blocks of infrequently accessed data then
it's a good idea to read it directly using :cpp:func:`flashmem_read`. You can get the address for a
memory location using :cpp:func:`flashmem_get_address`.

See :doc:`/framework/core/pgmspace` for details of how to store data in flash, and access it.
