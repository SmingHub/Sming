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
in the same 1MB block of flash. Set the following options in your project's
``component.mk`` file:

.. code-block:: make

   RBOOT_BIG_FLASH = 0
   RBOOT_TWO_ROMS  = 1
   SPI_SIZE        = 1M

SPIFFS
======

To use SPIFFS think about where you want your SPIFFS to sit on the flash.

If you have a 4MB flash the default position is for the first ROM
to be placed in the first 1MB block and the second ROM to be placed in
the third 1MB block of flash. This leaves a whole 1MB spare after each
ROM in which you can put your SPIFFS.

If you have to a smaller flash the SPIFFS will have to share the 1MB block with the ROM.
 For example, the first part of each 1MB block may contain the ROM, and the second part
the SPIFFS (but does *not* have to be split equally in half). So for the 4MB example
you could put the SPIFFS for your first ROM at flash address at 0x100000
and the SPIFFS for your second ROM at 0x300000; in each case that is the
1MB block after the ROM.

To mount your SPIFFS at boot time add the following code to init:

.. code-block:: c++

   int slot = rboot_get_current_rom();
   uint32_t address = (slot == 0) ? RBOOT_SPIFFS_0 : RBOOT_SPIFFS_1;
   //debugf("trying to mount SPIFFS at %x, length %d", address, SPIFF_SIZE);
   spiffs_mount_manual(address, SPIFF_SIZE);

Over-the-air (OTA) updates
==========================

Add the following code:

.. code-block:: c++

   RbootHttpUpdater* otaUpdater = nullptr;

   void OtaUpdate_CallBack(RbootHttpUpdater& client, bool result)
   {
       if (result) {
           // success - switch slot
           uint8_t slot = rboot_get_current_rom();
           if (slot == 0) {
               slot = 1;
           } else {
               slot = 0;
           }
           // set to boot new ROM and then reboot
           Serial.printf("Firmware updated, rebooting to ROM %d...\r\n", slot);
           rboot_set_current_rom(slot);
           System.restart();
       } else {
           // fail
           Serial.println("Firmware update failed!");
       }
   }

   void OtaUpdate()
   {
       // need a clean object, otherwise if run before and failed will not run again
       delete otaUpdater;
       otaUpdater = new RbootHttpUpdater();
       
       // select ROM slot to flash
       rboot_config bootconf = rboot_get_config();
       uint8_t slot = bootconf.current_rom;
       if (slot == 0) {
           slot = 1;
       } else {
           slot = 0;
       }

   #ifndef RBOOT_TWO_ROMS
       // flash ROM to position indicated in the rBoot config ROM table
       otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
   #else
       // flash appropriate ROM
       otaUpdater->addItem(bootconf.roms[slot], (slot == 0) ? ROM_0_URL : ROM_1_URL);
   #endif

       // use user supplied values (defaults for 4MB flash in makefile)
       otaUpdater->addItem((slot == 0) ? RBOOT_SPIFFS_0 : RBOOT_SPIFFS_1, SPIFFS_URL);

       // set a callback
       otaUpdater->setCallback(OtaUpdate_CallBack);

       // start update
       otaUpdater->start();
   }

You will need to define ``ROM_0_URL``, ``ROM_1_URL`` and ``SPIFFS_URL``
with http urls for the files to download.
