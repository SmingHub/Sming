Adafruit ILI9341 Display
========================

This is a Sming port of the library for the Adafruit ILI9341 display products.

This library works with the Adafruit 2.8" Touch Shield V2 (SPI)
http://www.adafruit.com/products/1651
 
Check out the links above for our tutorials and wiring diagrams.
These displays use SPI to communicate, 4 or 5 pins are required
to interface (RST is optional).

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistribution


Configuration variables
-----------------------


.. envvar:: TFT_CS_PIN

   Pin to use for SPI CS (Chip Select). Active low.

.. envvar:: TFT_DC_PIN

   Pin for controlling DC (Data / Command) line

.. envvar:: TFT_RESET_PIN

   Pin for resetting the display. Optional.
