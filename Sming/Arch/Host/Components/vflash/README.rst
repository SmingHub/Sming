Virtual Flasher
===============

Flash memory access is emulated using :component-host:`spi_flash`, and this Component implements make targets to
operate on the flash backing file in a similar way to flashing a real device.

The following options are interpreted and used to provide command-line paramters to the emulator executable:

* ``FLASH_BIN``: full path to flash backing file
* ``SPI_SIZE``: Size of flash memory, e.g. 4M = 4MBytes, 512K = 512 KBytes, etc.
* ``HOST_FLASH_OPTIONS``: This defaults to a combination of the above variables, but you can override if necessary.
