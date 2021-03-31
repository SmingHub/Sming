Random Restarts
===============

First try setting the baud rate to ``74880``. Example for Linux::

   python -m serial.tools.miniterm /dev/ttyUSB0 74880

The random symbols should become readable messages.

If you see repeating messages containing ``rf_cal[0] !=0x05`` then most
probably you should ``initialize`` the flash memory on your ESP8266
device.

To achieve this do the following:

1) Check the :ref:`hardware_config` especially ``flash_size`` setting.

2) Run ``flashinit``::

      cd $SMING_HOME/../samples/Basic_Blink
      make flashinit

   This resets the flash memory to a default state, erasing any existing
   application, configuration or data stored there.

3) Re-program your device::

   make flash
