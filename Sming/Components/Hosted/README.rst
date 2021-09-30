HostEd
======

.. highlight:: bash

The hosted component allows Sming's host emulator to run parts of the commands on an actual microcontroller.
The communication is done via `simplePRC <https://simplerpc.readthedocs.io/>`_ and the microcontroller has to be flashed with a special application.

Overview
--------
Sming's host emulator allows easier debugging and development of embedded applications. This component named "Hosted" extends the host emulator
and facilitates testing functionality that only a real microcontroller can provide as for example digital I/O operations or SPI operations.

For example in order to run the Basic_Blink application under the host emulator and run the actual blinking of a LED on a microcontroller
we can compile the application using the following directives::

   make SMING_ARCH=Host ENABLE_HOSTED=tcp HOSTED_SERVER_IP=192.168.4.1

`SMING_ARCH=Host` instructs the build system to build the application for the Host architecture.
`ENABLE_HOSTED=tcp` instructs the host emulator to communication with the real microcontroller using TCP
`HOSTED_SERVER_IP=192.168.4.1` instructs the host emulator to connect to IP `192.168.4.1`.

We need to compile and flash also a special application on the desired microcontroller.
This application will act as an RPC Server and will execute the commands from the host emulator on the microcontroller.

In the sub-directory ``samples`` inside this directory you will find the sample applications that will turn your microcontroller into
RCP server.

The compilation and flashing for ESP32, for example, can be done using the following commands::

   cd samples/tcp
   make SMING_ARCH=Esp32 WIFI_SSID=YourSSID WIFI_PWD=YourPassword
   make flash

If you replace ``SMING_ARCH=Esp32`` with ``SMING_ARCH=Esp8266`` then the hosted application will be compiled and flashed on a ESP8266 microcontroller.
Make sure to replace the values of  WIFI_SSID and WIFI_PWD with the actual name and password for the Access Point (AP).

Communication
-------------
At the moment the communication between an application running on the Host and the RCP server running on a microcontroller
can be done using TCP or serial interface.
The ``transport`` classes are located under ``include/Hosted/Transport``.

Configuration
-------------
.. envvar:: ENABLE_HOSTED

   Default: empty (disabled)

   Enables the hosted component. Valid values for the moment are:
   - tcp - for communication over TCP network.
   - serial - for communication over serial interface

.. envvar:: HOSTED_SERVER_IP

   Default: 192.168.13.1

   Used only when ENABLE_HOSTED=tcp is specified. Specifies the IP address of the remote RPC server.

.. envvar:: HOSTED_COM_PORT

   Default: :envvar:`COM_PORT`

   Used only when ENABLE_HOSTED=serial is specified. Specifies which local communication port should be used to connect to the remote RPC server.

.. envvar:: HOSTED_COM_SPEED

   Default: 115200

   Used only when ENABLE_HOSTED=serial is specified. Specifies the communication baud rate.
