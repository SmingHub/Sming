**Introduction**

rBoot is an open source bootloader for the ESP8266, which is now fully
integrated with Sming. rBoot is a more flexible alternative to the
closed source bootloader supplied by Espressif with the SDK. A
bootloader allows you to have more than one application on the esp8266,
either completely different apps, or different version of the same app,
which can be updated over the air.

The example Basic_rBoot demonstrates the use of rBoot, but if you want
to add it to an existing project this little tutorial will guide you.

**Need to know**

-  The esp8266 can only memory map 1mb of flash at a time, your rom must
   fit within the a single 1mb block of flash.
-  Different 1mb blocks can be mapped at different times, so your rom
   does not have to be within the first 1mb block. The support for this
   in rBoot is called *big flash mode*. A single compiled and linked rom
   image can be placed at the same relative position in any 1mb block of
   flash.
-  If you have two smaller roms in a single 1mb of flash (your only
   option if you flash is 1mb or smaller) this is referred to as *two
   rom mode* in rBoot. Each rom needs to be separately linked to have
   the correct memory mapped flash offset. Examples are given below for
   common scenarios.

**Setting up build environment**

-  Make sure your ``Makefile`` has the following code at the bottom, if
   you have copied a sample since Sming v1.3.0 this will already be
   there:

.. code:: make

   # Include main Sming Makefile
   ifeq ($(RBOOT_ENABLED), 1)
   include $(SMING_HOME)/Makefile-rboot.mk
   else
   include $(SMING_HOME)/Makefile-project.mk
   endif

-  Add the following variables to ``Makefile-user.mk``:

.. code:: make

   #### overridable rBoot options ####
   ## use rboot build mode
   RBOOT_ENABLED ?= 1
   ## enable big flash support (for multiple roms, each in separate 1mb block of flash)
   RBOOT_BIG_FLASH ?= 1
   ## two rom mode (where two roms sit in the same 1mb block of flash)
   RBOOT_TWO_ROMS  ?= 0
   ## size of the flash chip
   SPI_SIZE        ?= 4M
   ## output file for first rom (.bin will be appended)
   #RBOOT_ROM_0     ?= rom0
   ## input linker file for first rom
   #RBOOT_LD_0      ?= rom0.ld
   ## these next options only needed when using two rom mode
   #RBOOT_ROM_1     ?= rom1
   #RBOOT_LD_1      ?= rom1.ld
   ## size of the spiffs to create
   #SPIFF_SIZE      ?= 65536
   ## option to completely disable spiffs
   DISABLE_SPIFFS  = 1
   ## flash offsets for spiffs, set if using two rom mode or not on a 4mb flash
   ## (spiffs location defaults to the mb after the rom slot on 4mb flash)
   #RBOOT_SPIFFS_0  ?= 0x100000
   #RBOOT_SPIFFS_1  ?= 0x300000
   ## esptool2 path
   #ESPTOOL2        ?= esptool2

-  Copy ``rom0.ld`` from the Basic_rBoot sample to your project.

**Building**

-  The options above are perfect for a 4mb flash, if this is what you
   have (e.g. an esp-12) you don’t need to do anything else. Otherwise
   see information below about configuration.
-  Run ``make`` as normal, rBoot and your app rom will both be built for
   you.
-  Running ``make flash`` will flash rBoot and the first rom slot.

**Configuring for two rom mode**

If you have a 1mb flash, you will need to have two 512kb rom slots, both
in the same 1mb block of flash. Set the following options in
``Makefile-user.mk``

.. code:: make

   RBOOT_ENABLED   ?= 1
   RBOOT_BIG_FLASH ?= 0
   RBOOT_TWO_ROMS  ?= 1
   SPI_SIZE        ?= 1M

**Spiffs**

To use spiffs think about where you want your spiffs to sit on the
flash. If you have a 4mb flash the default position is for the first rom
to be placed in the first 1mb block and the second rom to be placed in
the third 1mb block of flash. This leaves a whole 1mb spare after each
rom in which you can put your spiffs. If you have to a smaller flash the
spiffs will have to share the 1mb block with the rom e.g. first part of
each 1mb block contains the rom and the second part contains the spiffs
(does *not* have to be split equally in half). So for the 4mb example
you could put the spiffs for your first rom at flash address at 0x100000
and the spiffs for your second rom at 0x300000 (in each case that is the
1mb block after the rom).

To mount your spiffs at boot time add the following code to init:

.. code:: c

   int slot = rboot_get_current_rom();
   if (slot == 0) {
       //debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
       spiffs_mount_manual(RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
   } else {
       //debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
       spiffs_mount_manual(RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
   }

The addition of 0x40200000 is due to a Sming quirk, just ignore it.

**Over-the-air (OTA) updates**

Add the following code:

.. code:: c

   rBootHttpUpdate* otaUpdater = 0;

   void OtaUpdate_CallBack(rBootHttpUpdate& client, bool result) {
       if (result == true) {
           // success
           uint8 slot;
           slot = rboot_get_current_rom();
           if (slot == 0) slot = 1; else slot = 0;
           // set to boot new rom and then reboot
           Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
           rboot_set_current_rom(slot);
           System.restart();
       } else {
           // fail
           Serial.println("Firmware update failed!");
       }
   }

   void OtaUpdate() {

       uint8 slot;
       rboot_config bootconf;

       // need a clean object, otherwise if run before and failed will not run again
       if (otaUpdater) delete otaUpdater;
       otaUpdater = new rBootHttpUpdate();
       
       // select rom slot to flash
       bootconf = rboot_get_config();
       slot = bootconf.current_rom;
       if (slot == 0) slot = 1; else slot = 0;

   #ifndef RBOOT_TWO_ROMS
       // flash rom to position indicated in the rBoot config rom table
       otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
   #else
       // flash appropriate rom
       if (slot == 0) {
           otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
       } else {
           otaUpdater->addItem(bootconf.roms[slot], ROM_1_URL);
       }
   #endif

       // use user supplied values (defaults for 4mb flash in makefile)
       if (slot == 0) {
           otaUpdater->addItem(RBOOT_SPIFFS_0, SPIFFS_URL);
       } else {
           otaUpdater->addItem(RBOOT_SPIFFS_1, SPIFFS_URL);
       }

       // set a callback
       otaUpdater->setCallback(OtaUpdate_CallBack);

       // start update
       otaUpdater->start();
   }

You will need to define ``ROM_0_URL``, ``ROM_1_URL`` and ``SPIFFS_URL``
with http urls for the files to download.
