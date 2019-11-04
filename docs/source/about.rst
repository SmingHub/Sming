About Sming
===========

ESP8266
-------

This is the documentation for the API of Sming, a C++ framework for development on the ESP8266 microcontroller platform.

The `ESP8266 <https://www.espressif.com/en/products/hardware/esp8266ex/overview>`__ is a microcontroller with Wi-Fi,
manufactured by `Espressif <https://espressif.com>`__ Systems.

The microcontroller is used in `modules <https://en.wikipedia.org/wiki/ESP8266>`__ from various manufacturers,
each of which present a subset of the interface pins.
For example, the ESP-01 only provides 2 digital GPIO whereas the ESP-12 provides 9 digital GPIO plus ADC analogue input.

Sming provides a C++ framework that facilitates the creation of application code for the ESP8266.
The core framework provides access to the ESP8266 functions such as GPIO, timers, WiFi configuration, etc.
A set of libraries extend this functionality such as switch debounce, SD Card interface, etc.

Licenses
--------

Sming Core
   LGPL v3

Espressif SDK
   ESPRSSIF MIT License (with some closed-source blobs)

Libraries
   See each library for details of its own open source license
