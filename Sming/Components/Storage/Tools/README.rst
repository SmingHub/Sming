Partition tools
===============

hwconfig.py
   Supports parsing and creation of hardware configuration files.


Notes
-----

Comparison between ESP-IDF and ESP8266-RTOS versions:

gen_esp32part.py
   Same name but slightly different:
      ESP32 application partitions are aligned to 0x10000 boundary, ESP8266 to 0x1000

   This is the ESP32 version.
   Consider modifying,  e.g add paramter to change alignment.

parttool.py
   From ESP-IDF. More recent, a few exception handling tweaks.

gen_empty_partition.py
   Identical.

