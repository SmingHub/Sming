Flash memory
============

.. highlight:: C++

Introduction
------------

ESP8266 flash memory sizes vary from 512Kbytes on the ESP-01 up to 4Mbytes on the ESP12F.
Up to 16MBytes are supported for custom designs.
Rp2040 has similar support, and the Esp32 has enhanced VMM (Virtual Memory Management) hardware.

Sming version 4.3 introduced partition tables to support multiple architectures,
different hardware variants and custom flash layouts without restriction.
It is binary compatible with the Esp32 IDF partition tables but with a consistent API across all architectures.

See :ref:`hardware_config` for details.

A typical layout for a 4MByte Esp8266 device might look like this:

   =======  ===============   ====   =========================  ===================================================                                
   Address  Config variable   Size   Source filename            Description            
   (hex)    (if any)          (KB)   (if applicable)            
   =======  ===============   ====   =========================  ===================================================            
   000000                     1      rboot.bin                  Boot loader            
   001000                     4                                 rBoot configuration            
   002000   ROM_0_ADDR               rom0.bin                   First ROM image            
   102000   ROM_1_ADDR               rom1.bin                   Second ROM image            
   200000   RBOOT_SPIFFS_0
   300000   RBOOT_SPIFFS_1
   3FA000                     4                                 Partition table
   3FB000                     4      blank.bin                  RF Calibration data (Initialised to FFh)
   3FC000                     4      esp_init_data_default.bin  PHY configuration data            
   3FD000                     12     blank.bin                  System parameter area
   =======  ===============   ====   =========================  ===================================================            
   

.. note::

   This was the previous layout for a 4MByte flash device:

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

The actual layout in use can be seen by running ``make map``.


Speed and caching
-----------------

Flash memory is accessed via an external SPI bus, so reading it takes about 12x
longer than reading from internal RAM. To mitigate this, some of the internal RAM is used to
cache data. Part of this is managed in hardware, which means if the data required is already in
the cache then there is no difference in speed. In general, then, frequently accessed data is read
as if it were already in RAM.

Bear in mind that every time new data is read via the cache, something else will get thrown away
and have to be re-read. Therefore, if you have large blocks of infrequently accessed data then
it's a good idea to read it directly using :cpp:func:`flashmem_read`. You can get the address for a
memory location using :cpp:func:`flashmem_get_address`.

See :doc:`/framework/core/pgmspace` for details of how to store data in flash, and access it.
