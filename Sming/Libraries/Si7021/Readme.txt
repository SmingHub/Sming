This library was made for the Si7021 Humidity and Temperature sensor and only tested
on the teensy 3.1 but it should work for Arduino also. The Si7021 is an I2C device
that runs on 3.3v, it has four pins broken out, two for power and two for I2C so
connecting the sensor is pretty straight forward. The Sensor has good accuracy and
is fairly cheap on ebay to buy on a breakout board with pin headers. The sensor offers
some nice features, to name a few...

: 12-bit Humidity Resolution 0.025
: ±3% RH Humidity Accuracy
: 14-bit Temperature Resolution 0.01
: ±0.4°C Temperature Accuracy
: Factory Calibrated (interchangeable)
: Integrated Heater
: Low Power - 150μA active / 60nA standby 

The library uses all the features available to the sensor which is listed in the
datasheet, except one, setting the heater power. There is a problem with writing to
and reading from the heater control register which is the register to set how much
current will be used by the heater. Turning the actual heater on or off is done in
another register so the heater can be enabled and disabled, but only the lowest value
of 3.09mA can be used as this is the default heater power.

The HTRE bit in the user register is what turns the heater on and off. This register
is stored on non-volatile memory to it will keep its state when power is removed. If
the heater is enabled and power is removed before the program had chance to turn it off
then the heater will remain enabled the next time it is powered on. The heater has to
be explicitly turned off. In the begin() function for this library, a reset procedure
will take place and reset the user register to default, so the heater will be turned off
and the resolution will also be set to default, 14-bit temperature and 12-bit humidity.
Keep this in mind if testing and swapping sensors/libraries.

There are two functions to change the resolution, setHumidityRes(uint8_t) and
setTempRes(uint8_t). When setting a resolution with one function it will change
the other resolution also. This is because the sensor register uses two bits to set the
resolution for both humidity and temperature so when changing one the other must also
change. The two functions have similar functionality except they have different bit masks
to the register for a given resolution depending if humidity or temp resolution is to be
changed. Below is a table of the relationship between humidity and temperature
resolution...

Resolution Table: 14-bit Temp <-> 12-bit Humidity
                  13-bit Temp <-> 10-bit Humidity
                  12-bit Temp <->  8-bit Humidity
                  11-bit Temp <-> 11-bit Humidity



The library also includes some conversion functions to change the temperature from
Celsius to Fahrenheit and Fahrenheit to Celsius. There are also two other functions
which allow to change the humidity or temperature from a float to an int or from an int
to a float. The library uses floats for humidity and temperature reading, so these
functions provide a way to convert to an int if needed.

The library comes with three examples which print information to the serial monitor.
The following gives a brief description of the examples...


Basic_Demo : Print humidity and temperature to the serial monitor while toggling
             the heater on and off periodically.

Functions : This prints a demo of the function use to the serial monitor, showing what
            action to expect, the status/return of the function and the function called.

Timings : Prints information to the serial monitor showing how long each function
          takes to return. The timing is done in microseconds.


In the zip file provided which contains this library will also be a copy of the
datasheet and a text file named (Functions_Info.txt) that has a complete list of
functions and some info on what the function does, what it returns and possible
parameters for certain functions and a short line of extra info.

I hope this library will help someone with their project.

Si7021 Library - DOWNLOAD



