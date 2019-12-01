rBoot
=====

rBoot is a 2nd stage bootloader that allows booting application images from several 
pre-configured flash memory addresses (called "slots"). Sming supports up to three 
slots.

Sming uses rBoot exclusively because of its flexibility, reliability and ease of use. 

*Warning note: Make sure that your slots do not extend beyond a 1MB boundary and 
do not overlap with each other, the file system (if enabled) or the RFcal/system 
parameters area! Sming currently has no means of detecting a misconfigured memory 
layout.*

Slot 0 at `RBOOT_ROM0_ADDR`
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the default slot which is always used. Its start address can be configured
via `RBOOT_ROM0_ADDR`. The default is 0x2000, which is the third sector, right after 
rBoot and its configuration data. Except for the use case described in the "Slot 2" 
section below, you should hardly ever need to change this default.

Slot 1 at `RBOOT_ROM1_ADDR`
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This slot is disabled by default. If you don't need it, just leave `RBOOT_ROM1_ADDR` 
unconfigured (empty).

However, if your application includes any kind of Over-the-Air (OTA) firmware 
update functionality, you will need a second memory slot to store the received 
update image while the update routines execute from the first slot. Upon successful 
completion of the update, the second slot is activated, such that on next reset 
rBoot boots into the uploaded application. While now running from slot 1, the next 
update will be stored to slot 0 again, i.e. the roles of slot 0 and slot 1 are 
flipped with every update.

The start address of slot 1 is configured by `RBOOT_ROM1_ADDR`. For devices with 
more than 1MB of flash memory, it is advisable to choose an address with the same 
offset within its 1MB block than `RBOOT_ROM0_ADDR`, e.g. 0x102000 for the default 
slot 0 address (0x2000). This way, the same application image can be used for both 
slots (see `RBOOT_TWO_ROMS` below for further details).


Slot 2 (GPIO slot) at `RBOOT_ROM2_ADDR`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This slot is also disabled by default.

rBoot supports booting into a third slot upon explicit user request, e.g. by pressing 
a button during reset/power up. This is especially useful for implementing some sort 
of recovery mechanism. To enable slot 2, set `RBOOT_GPIO_ENABLED` to 1 and `RBOOT_ROM2_ADDR` 
to a suitable address. Note that these settings will only configure rBoot. Sming will 
not create an application image for slot 2. You can, however, use a second Sming 
project to build a recovery application image as follows:

1. Create a new Sming project for your recovery application. This will be a simple
   single-slot project. Set `RBOOT_ROM0_ADDR` of the recovery project to the value 
   of `RBOOT_ROM2_ADDR` of the main project.

2. Build and flash the recovery project as usual by typing `make flash`. This will 
   install the recovery ROM (into slot 2 of the main project) and a temporary 
   bootloader, which will be overwritten in the next step.

3. Go back to your main project. Build and flash it with `make flash`. This will 
   install the main application (into slot 0) and the final bootloader. You are 
   now all set for booting into the recovery image if the need arises.

Automatically derived settings
==============================

The following items where configuration variables in earlier versions of Sming and 
are now derived automatically, i.e. it is no longer necessary nor possible to 
configured them from your project:

* `RBOOT_BIG_FLASH`
* `RBOOT_TWO_ROMS`


Big Flash Mode (`RBOOT_BIG_FLASH`)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ESP8266 can map only 1MB of flash memory into its internal address space at a time. 
As you might expect, the first megabyte is mapped by default. This is fine if your image(s) 
reside(s) in this range. Otherwise, rBoot has to inject a piece of code into the 
application startup routine to set up the flash controller with the correct mapping. 
The Sming build system automatically enables Big Flash Mode when `RBOOT_ROM0_ADDR` 
or `RBOOT_ROM1_ADDR` >= `0x100000`.

Single vs. Dual ROMs (`RBOOT_TWO_ROMS`)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since every 1MB range of flash memory is mapped to an identical internal address range, 
the same ROM image can be used for slots 0 and 1 **if (and only if!)** both slots have 
the same address offsets within their 1MB blocks, i.e. `(RBOOT_ROM0_ADDR & 0xFFFFF) == 
(RBOOT_ROM1_ADDR & 0xFFFFF)`. 
Consequently, for such configurations, the Sming build system generates only one ROM image.

In all other cases, two distinct application images must be linked with different addresses 
for the 'irom0_0_seg' memory region. The Sming build system takes care of all the details, 
including linker script generation from a template (`RBOOT_LD_TEMPLATE`). Just run `make` 
and find the resulting ROM images in the 'firmware' folder of your project's output directory.


Further configuration settings
==============================

.. envvar:: ESPTOOL2

   Defines the path to the esptool2 tool which rBoot uses to manipulate ROM
   images. Use ``$(ESPTOOL2)`` if you need it within your own projects.