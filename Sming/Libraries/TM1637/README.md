TM1637
======
Arduino library for TM1637 (LED Driver)


Description
-----------
An Arduino library for 7-segment display modules based on the TM1637 chip, such as Seeed Studio's [Grove 4 digit display](http://www.seeedstudio.com/depot/grove-4digit-display-p-1198.html). The TM1637 chip also has keyboard input capability, but it's not implemented in this library.

Hardware Connection
-------------------
The display modules has two signal connection (and two power connections) which are CLK and DIO. These pins can be connected to any pair of digital pins on the Arduino. When an object is created, the pins should be configured. There is no limitation on the number of instances used concurrently (as long as each instance has a pin pair of its own)

Installation
------------
The library is installed as any Arduino library, by copying the files into a directory on the library search path of the Arduino IDE

Usage
-----
The library provides a single class named TM1637Display. An instance of this class provides the following functions:

* `setSegments` - Set the raw value of the segments of each digit
* `showNumberDec` - Display a decimal number
* `showNumberDecEx` - Display a decimal number with decimal points or colon
* `setBrightness` - Sets the brightness of the display

The information given above is only a summary. Please refer to TM1637Display.h for more information. An example is included, demonstrating the operation of most of the functions.
