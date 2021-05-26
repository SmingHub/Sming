Basic Ota
=========

.. highlight:: bash

Introduction
------------

This sample integrates :component:`Ota`, :component:`OtaNetwork` and Sming.
It demonstrates dual rom booting, big flash support, OTA updates and dual spiffs filesystems.
This sample should work on all supported architectures.

Esp8266
~~~~~~~
On Esp8266 we use rBoot as bootloader.
When using rBoot big flash support with multiple 1MB slots only one rom
image needs to be created. If you don’t want to use big flash support
(e.g. for a device with smaller flash) see the separate instructions
below. You can easily take the ota files and add them to your own
project to add OTA support.

Building
--------

1) Set :envvar:`WIFI_SSID` & :envvar:`WIFI_PWD` environment variables with your wifi details.
2) Edit the OTA server details defined in the application ``component.mk`` file.
3) ``make && make flash``
4) Put *rom0.bin* and *spiff_rom.bin* in the root of your webserver for OTA.
5) Interact with the sample using a terminal (``make terminal``). Sorry - no web-gui (yet).


Testing
-------

For testing purposes we provide an Ota server that can be started on your desktop machine::

   make otaserver

The server listens on port 9999 and all network interfaces. If your desktop has the following IP address ``192.168.1.30``
after connecting to your WIFI router then you can compile the sample to use this IP address and the testing OTA server::

   make ROM_0_URL=http://192.168.1.30:9999/rom0.bin SPIFFS_URL=http://192.168.1.30:9999/spiff_rom.bin
   make flash

Make sure to replace ``192.168.1.30`` with your WIFI IP address.

Flash layout considerations
---------------------------

Esp8266
~~~~~~~
If you want to use, for example, two 512k roms in the first 1MB block of
flash (old style) then Sming will automatically create two separately linked
roms. If you are flashing a single rom to multiple 1MB flash blocks, all using
the same offset inside their 1MB blocks, only a single rom is created.
See :component:`rboot` for further details.

-  If using a very small flash (e.g. 512k) there may be no room for a
   spiffs fileystem, so use *HWCONFIG = standard*
-  After building copy all the rom*.bin files to the root of your web
   server.

If you want more than two roms you must be an advanced user and should
be able to work out what to copy and edit to acheive this!

Configuration
-------------
.. envvar:: RBOOT_TWO_ROMS

   Default: 1 (enabled)

   Allows specifying two different URLs for ROM0 and ROM1.

   If not set then only the URL defined in ROM_0_URL will be used.

.. envvar:: ROM_0_URL

   Default: http://192.168.7.5:80/rom0.bin

   The URL where the firmware for the first application partition can be downloaded.

.. envvar:: ROM_1_URL

   Default: http://192.168.7.5:80/rom1.bin

   Used when ``RBOOT_TWO_ROMS`` is set. The URL where the firmware for the second application partition can be downloaded.

.. envvar:: SPIFFS_URL

   Default: http://192.168.7.5:80/spiff_rom.bin

   The URL where the spiffs partition attached can be downloaded.


Credits
-------

The initial sample was made possible with the assistance of piperpilot,
gschmott and robotiko on the esp8266.com forum.
