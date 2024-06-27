About
=====

Sming
-----

Sming is an open-source asynchronous C++ embedded framework with networking support
and is designed to be reliable, responsive and modular.

The project was started in 2015 for the Esp8266 and has since been adapted to other devices,
including a :doc:`host emulator <arch/host/index>` for advanced development and debugging.

The framework is actively developed and has an extensive suite of integration tests.


Arduino
-------

Arduino is an open-source electronics platform based on easy-to-use hardware and software.
Sming is provided with a curated set of :doc:`/libraries`, including many standard Arduino-style ones.
Lost of popular hardware can be used in a few lines of code.


ESP8266
-------

`ESP8266 <https://www.espressif.com/en/products/socs/esp8266>`__ is a microcontroller with Wi-Fi,
manufactured by Espressif Systems.
It is the first microcontroller that was supported from Sming.
Sming provides access to all ESP8266 functions such as GPIO, timers, WiFi configuration, etc.


ESP32
-----

`ESP32 <https://www.espressif.com/en/products/socs/esp32>`__ is the second microcontroller by `Espressif <https://espressif.com>`__ Systems.
There are also a number of more recent variants such as the esp32-s2 and esp32-c3/s3 (RISCV) which are supported by Sming.

The Esp32 WiFi stack requires FreeRTOS so Sming applications also have to run on this platform.
However, Sming applications should avoid making use of FreeRTOS-specific features where possible
as this reduces portability to other devices.


RP2040
------

The `RP2040 <https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html>`__ is the Raspberry Pi Foundation's first microcontroller.
It is an ARM-based device with dual cores and a good set of hardware peripherals.
Perhaps its most outstanding feature is programmable-IO which allows high-performance protocols to be implemented.

There are various development boards available, including the original Pico boards which have a networked option: the Pico-W.
This uses an external CYW43-based network SOC which handles all the low-level protocols.

The RP2040 SDK is simple, reliable of high-quality and is fully open-source, including the bootloader ROM code.
There is no FreeRTOS complexity to deal with and is an ideal foundation for Sming applications.


Licenses
--------

Sming Core
   LGPL v3

Espressif Non-OS SDK for Esp8266
   ESPRESSIF MIT License (with some closed-source blobs)

Libraries
   See each library for details of its own open source license
