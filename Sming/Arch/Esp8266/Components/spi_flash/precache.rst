IRAM pre-caching
================

Introduction
------------

This is taken from the esp8266 arduino core. For details see:

- https://github.com/esp8266/Arduino/pull/6628
- https://github.com/esp8266/Arduino/issues/6559

See :doc:`/information/flash` for some background.

Code is always executed from IRAM, however it must be read in from flash memory on
first use which can cause issues within timing-critical code.

This can be avoided by placing the code in IRAM.
However, IRAM is a very limited resource and should generally be reserved
for interrupt service routines.

An alternative solution is to arrange for critical code to be pre-loaded into IRAM
before it starts execution.


.. note::

   This cannot be used for interrupt service routines.

   By their nature, interrupts are essentially random events and therefore code
   must be available in IRAM at any time.

Usage
-----

The steps required are:

1. Mark the function containing code using :c:func:`IRAM_PRECACHE_ATTR`
2. Place a call to :c:func:`IRAM_PRECACHE_START` *before* the first line of critical code
3. Place a call to :c:func:`IRAM_PRECACHE_END` *after* the last line of critical code

You must always declare a tag to avoid the risk of section name conflicts.

You can find an example of how precaching is used here:

https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_spi_utils.cpp

API Documentation
-----------------

.. doxygengroup:: iram_precache
   :content-only:
