Random Restarts
===============

First try setting the baud rate to ``74880``. Example for Linux:

::

   python -m serial.tools.miniterm /dev/ttyUSB0 74880

The random symbols should become readable messages.

If you see repeating messages containing ``rf_cal[0] !=0x05`` then most
probably you should ``initialize`` the flash memory on your ESP8266
device.

To achieve this do the following:

1) Set the SPI_SIZE of your flash memory. Example: If you have device
   with 4 megabytes make sure that the following is set:

::

   export SPI_SIZE=4M

2) Run ``flashinit``.

Run the following commands.

::

   cd $SMING_HOME/../samples/Basic_Blink
   make flashinit

``flashinit`` is erasing the current flash memory and populating some
areas on it with the bytes that your SDK / BootROM is expecting to be
present.

This command needs to be executed only when you change SDKs or memory
layouts.
