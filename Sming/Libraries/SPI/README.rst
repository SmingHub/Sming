SPI Library
===========

Provides support for both hardware and software-base SPI master devices.

Software SPI is likely to be of use only for ESP8266, but is enabled for all architectures.

Build variables
---------------

.. envvar:: ENABLE_SPISOFT_DELAY

   default: 0 (disabled)

   The ESP8266 can achieve a minimum of about 70ns between bit edges, with a maximum of
   about 1MBit/s at normal CPU clock frequency or 2MHz if CPU clock set to fast.

   To accommodate slower SPI devices this setting should be enabled and a suitable value
   provided for the ``delay`` parameter (typically 1 or 2).

   The appropriate delay factor can be provided in the :cpp:class:`SPISoft` constructor,
   or by calling :cpp:func:`SPISoft::setDelay`.

   Note that delay=0 is not the same as disabling delays.
