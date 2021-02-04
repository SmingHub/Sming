*********************
rBoot and OTA updates
*********************

Introduction
============

rBoot is an open source bootloader for the ESP8266, which is now fully
integrated with Sming. rBoot is a more flexible alternative to the
closed source bootloader supplied by Espressif with the SDK. A
bootloader allows you to have more than one application on the esp8266,
either completely different apps, or different version of the same app,
which can be updated over the air.

The example :sample:`Basic_rBoot` demonstrates the use of rBoot, but if you want
to add it to an existing project this little tutorial will guide you.

Need to know
============

-  The esp8266 can only memory map 1MB of flash at a time, your ROM must
   fit within the a single 1MB block of flash.
-  Different 1MB blocks can be mapped at different times, so your ROM
   does not have to be within the first 1MB block. The support for this
   in rBoot is called *big flash mode*. A single compiled and linked ROM
   image can be placed at the same relative position in any 1MB block of
   flash.
-  If you have two smaller ROMs in a single 1MB of flash (your only
   option if you flash is 1MB or smaller) this is referred to as
   *two ROM mode* in rBoot. Each ROM needs to be separately linked to have
   the correct memory mapped flash offset. Examples are given below for
   common scenarios.

Building
========

-  The default rBoot options are perfect for a 4MB flash, so if this is what you
   have (e.g. an esp-12) you don’t need to do anything else. Otherwise
   see information below about configuration.
-  Run ``make`` as normal, rBoot and your app ROM will both be built for you.
-  Running ``make flash`` will flash rBoot and the first ROM slot.

Configuring for two ROM mode
============================

If you have a 1MB flash, you will need to have two 512KB ROM slots, both
in the same 1MB block of flash. You can accommodate this by setting the
appropriate hardware configuration in your project's component.mk file::

.. code-block:: make

   HWCONFIG = two-rom-mode

See ``Sming/Arch/Esp8266/two-rom-mode.hw`` for details.
You can copy this and customise it in your project.


SPIFFS
======

To use SPIFFS think about where you want your SPIFFS to sit on the flash.

If you have a 4MB flash the default position is for the first ROM
to be placed in the first 1MB block and the second ROM to be placed in
the third 1MB block of flash. This leaves a whole 1MB spare after each
ROM in which you can put your SPIFFS. This is the behaviour when you
set ``HWCONFIG = spiffs`` in your project's component.mk file.

If you have a smaller flash the SPIFFS will have to share the 1MB block with the ROM.
For example, the first part of each 1MB block may contain the ROM, and the second part
the SPIFFS (but does *not* have to be split equally in half). So for the 4MB example
you could put the SPIFFS for your first ROM at flash address at 0x100000
and the SPIFFS for your second ROM at 0x300000; in each case that is the
1MB block after the ROM.

To mount your SPIFFS at boot time add the following code to init:

.. code-block:: c++

   int slot = rboot_get_current_rom();
   // Find the n'th SPIFFS partition
   auto part = PartitionTable().find(Partition::SubType::Data::spiffs, slot);
   if(part) {
      //debugf("trying to mount SPIFFS at %x, length %d", part.address(), part.size());
      spiffs_mount(part);
   } else {
      debug_e("SPIFFS partition missing for slot #%u", slot);
   }

Over-the-air (OTA) updates
==========================

Instead of insisting on a "one-solution-fits-all" approach, Sming provides you 
with the ingredients to build an OTA upgrade mechanism tailored to your application.
This involves selecting a transport protocol and a backend that interacts with the 
flash memory. Any protocol from Sming's rich set of network classes can be used, 
ranging from raw TCP sockets to HTTP, FTP, MQTT, with or without SSL, etc.
To conserve program memory, you might prefer a protocol already employed by your 
application.

On the backend side, there is :cpp:type:`OtaUpgradeStream` from the :library:`OtaUpgrade`
library, which supports multiple ROM images in one upgrade file, as well as 
state-of-the-art security features like a digital signatures and encryption. 
Check out the :sample:`HttpServer_FirmwareUpload` example, which demonstrates a
browser-based firmware upgrade mechanism similar to what is found in many consumer
products.
 
A more lightweight solution is provided by :cpp:class:`RbootOutputStream`, which 
is just a thin wrapper around rBoot's flash API, in combination with :cpp:class:`RbootHttpUpdater`,
which pulls individual ROM image from an HTTP server.

For details, refer to the `OtaUpdate()` function in the :sample:`Basic_rBoot` sample.
