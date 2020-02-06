rBoot
=====

Introduction
------------

rBoot is a second-stage bootloader that allows booting application images from several 
pre-configured flash memory addresses, called "slots". Sming supports up to three slots.

Sming uses rBoot exclusively because of its flexibility, reliability and ease of use. 

.. attention::

   Make sure that your slots do not extend beyond a 1MB boundary and 
   do not overlap with each other, the file system (if enabled) or the RFcal/system 
   parameters area! Sming currently has no means of detecting a misconfigured memory 
   layout.

Slot 0
------

This is the default slot which is always used.

.. envvar:: RBOOT_ROM0_ADDR

   default: 0x2000.

   This is the start address for slot 0. The default is sector 3, immediately after rBoot and its configuration data.

   Except for the use case described in `Slot2`_ below, you should not need to set this value.

Slot 1
------

.. envvar:: RBOOT_ROM1_ADDR

   default: disabled

   The start address of slot 1. If you don't need it, leave unconfigured (empty).

If your application includes any kind of Over-the-Air (OTA) firmware
update functionality, you will need a second memory slot to store the received
update image while the update routines execute from the first slot.

Upon successful completion of the update, the second slot is activated, such that on next reset
rBoot boots into the uploaded application. While now running from slot 1, the next
update will be stored to slot 0 again, i.e. the roles of slot 0 and slot 1 are
flipped with every update.

For devices with more than 1MB of flash memory, it is advisable to choose an address with the same 
offset within its 1MB block as :envvar:`RBOOT_ROM0_ADDR`, e.g. 0x102000 for the default 
slot 0 address (0x2000). This way, the same application image can be used for both 
slots. See :ref:`single_vs_dual` for further details.

.. _Slot2:

Slot 2 (GPIO slot)
------------------

rBoot supports booting into a third slot upon explicit user request, e.g. by pressing 
a button during reset/power up. This is especially useful for implementing some sort 
of recovery mechanism.

To enable slot 2, set these values:

.. envvar:: RBOOT_GPIO_ENABLED

   Disabled by default. Set to 1 to enable slot 2.

.. envvar:: RBOOT_ROM2_ADDR

   Address for slot 2

Note that this will only configure rBoot.
Sming will  not create an application image for slot 2.
You can, however, use a second Sming  project to build a recovery application image as follows:

1. Create a new Sming project for your recovery application. This will be a simple
   single-slot project. Set :envvar:`RBOOT_ROM0_ADDR` of the recovery project to the value 
   of :envvar:`RBOOT_ROM2_ADDR` of the main project.

2. Build and flash the recovery project as usual by typing ``make flash``. This will 
   install the recovery ROM (into slot 2 of the main project) and a temporary 
   bootloader, which will be overwritten in the next step.

3. Go back to your main project. Build and flash it with ``make flash``. This will 
   install the main application (into slot 0) and the final bootloader. You are 
   now all set for booting into the recovery image if the need arises.

Automatically derived settings
------------------------------

The :envvar:`RBOOT_BIG_FLASH` and :envvar:`RBOOT_TWO_ROMS` settings are now read-only
as their values are derived automatically.

In earlier versions of Sming these had to be set manually.

Big Flash Mode
~~~~~~~~~~~~~~

The ESP8266 can map only 1MB of flash memory into its internal address space at a time. 
As you might expect, the first megabyte is mapped by default. This is fine if your image(s) 
reside(s) in this range. Otherwise, rBoot has to inject a piece of code into the 
application startup routine to set up the flash controller with the correct mapping.

.. envvar:: RBOOT_BIG_FLASH

   **READONLY** Set when :envvar:`RBOOT_ROM0_ADDR` or :envvar:`RBOOT_ROM1_ADDR` >= 0x100000.

   See also :ref:`big_flash_support`.

.. _single_vs_dual:

Single vs. Dual ROMs
~~~~~~~~~~~~~~~~~~~~

Since every 1MB range of flash memory is mapped to an identical internal address range, 
the same ROM image can be used for slots 0 and 1 **if (and only if!)** both slots have 
the same address offsets within their 1MB blocks, i.e. ``(RBOOT_ROM0_ADDR & 0xFFFFF) == 
(RBOOT_ROM1_ADDR & 0xFFFFF)``.

Consequently, for such configurations, the Sming build system generates only one ROM image.

In all other cases, two distinct application images must be linked with different addresses 
for the 'irom0_0_seg' memory region. The Sming build system takes care of all the details, 
including linker script generation.

.. envvar:: RBOOT_TWO_ROMS

   **READONLY** Determines if rBoot needs to generate distinct firmware images.

Further Configuration Settings
------------------------------

.. envvar:: RBOOT_SILENT

   Default: 0 (verbose)

   At system restart rBoot outputs debug information to the serial port.
   Set to 1 to disable.


.. envvar:: RBOOT_LD_TEMPLATE

   Path to the linker script template. The actual script is output to the application build
   directory (e.g. ``rom0.ld``), replacing the `irom0_0_seg` entry according to the
   configured build settings.


.. envvar:: RBOOT_ROM_0

   Base name for firmware image #0. Default is ``rom0``.


.. envvar:: RBOOT_ROM_1

   Base name for firmware image #1. Default is ``rom1``.


.. envvar:: ESPTOOL2

   **READONLY** Defines the path to the esptool2 tool which rBoot uses to manipulate ROM
   images. Use ``$(ESPTOOL2)`` if you need it within your own projects.


API Documentation
-----------------

.. toctree::

   api
   Cache_Read_Enable
