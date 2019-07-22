Virtual Flasher
===============

Flash memory access is emulated using :component-host:`spi_flash`, and this Component implements make targets to
operate on the flash backing file in a similar way to flashing a real device.

The following options are interpreted and used to provide command-line parameters to the emulator executable:

.. envvar:: FLASH_BIN

   Full path to the flash backing file

.. envvar:: HOST_FLASH_OPTIONS

   This defaults to a combination of the above variables, but you can override if necessary.

There are additional settings defined in the Esp8266 :component-esp8266:`esptool`.
