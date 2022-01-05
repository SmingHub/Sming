SPI Library
===========

Provides support for both hardware and software-base SPI master devices.

Software SPI is likely to be of use only for ESP8266, but is enabled for all architectures.

Build variables
---------------

.. envvar:: SPISOFT_DELAY_VARIABLE

   default: 0 (disabled)

   This setting must be enabled in order to use :cpp:func:`SPISoft::setDelay`.
   The base clock speed (i.e. delay=0) is reduced to about 1.2 MBit/s (2 MBit/s for fast CPU).

   The appropriate delay factor can be provided in the :cpp:class:`SPISoft` constructor,
   or by calling :cpp:func:`SPISoft::setDelay`.

   With the ESP8266, then the appropriate delay factor is calculated automatically from
   the speed passed in :cpp:class:`SPISettings`.
   This will override any previously set delay factor.
   Use a speed of 0 to use the manually configured delay value.


.. envvar:: SPISOFT_DELAY_FIXED

   default: 0 (disabled)
   maximum: 10

   Adds the requested number of 'NOP' CPU instructions to every clock transition.

   Has less impact than enabling variable delays.

   Use variable delays if 10 is insufficient.

   Will be ignored if variable delays are enabled.



.. envvar:: ENABLE_SPI_DEBUG

   default: 0 (disabled)

   Enable to print additional debug messages.


API Documentation
-----------------

.. doxygengroup:: base_spi
   :content-only:
   :members:

.. doxygengroup:: hw_spi
   :content-only:
   :members:

.. doxygengroup:: soft_spi
   :content-only:
   :members:
