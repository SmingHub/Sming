Virtual Flasher
===============

Flash memory access is emulated using :component-host:`spi_flash`.

This Component implements make targets to operate on the flash backing file in a similar way to flashing a real device.


Configuration Variables
-----------------------

The following options are interpreted and used to provide command-line parameters to the emulator executable:

.. envvar:: FLASH_BIN

   Full path to the flash backing file

.. envvar:: HOST_FLASH_OPTIONS

   This defaults to a combination of the above variables, but you can override if necessary.

The size of the flash memory is set via :ref:`hardware_config`.

See :component:`esptool` for details and other applicable variables.


Build targets
-------------

-  ``make flashinit`` to clear and reset the file.
-  ``make flashfs`` to copy the generated SPIFFS image into the backing file.
-  ``make flash`` writes out all required images to the backing file.
   For now, this is the same as ``make flashfs`` but that will change
   when support is added for custom user images.

