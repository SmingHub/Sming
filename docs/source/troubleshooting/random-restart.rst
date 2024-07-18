Random Restarts
===============

Esp8266
-------

First try setting the baud rate to ``74880``::

   make terminal COM_SPEED=74880

The random symbols should become readable messages.

If you see repeating messages containing ``rf_cal[0] !=0x05`` then most
probably you should ``initialize`` the flash memory on your ESP8266 device.

To achieve this do the following:

1) Check the :ref:`hardware_config` especially ``flash_size`` setting.

.. note::

   If you're not sure what size the flash memory is on your device, run `make flashid` to check.

2) Run ``flashinit``::

      cd $SMING_HOME/../samples/Basic_Blink
      make flashinit

   This resets the flash memory to a default state, erasing any existing
   application, configuration or data stored there.

3) Re-program your device::

      make flash

4) Verify flash data has been written successfully

      make verifyflash


Esp32
-----

Bootloader messages should be visible and may give some indication as to the problem.

If :envvar:`SDK_CUSTOM_CONFIG` has been set in your project, or the SDK configuration has otherwise
been changed from defaults, try reverting it back to defaults::

   make sdk-config-clean
   make SDK_CUSTOM_CONFIG=

The ESP IDF framework runs on FreeRTOS and there is a known deadlock problem related to flash memory access if FreeRTOS is initialised and running on both cores.
For this reason, the setting ``CONFIG_FREERTOS_UNICODE=y`` must be set. See :issue:`2653`.
