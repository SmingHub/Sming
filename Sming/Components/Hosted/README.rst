HostEd
============

.. highlight:: bash

The hosted component allows Sming's host emulator to run parts of the commands on an actual microcontroller.
The communication is done via protobuf messages and the microcontroller has to be flashed with a special application.

Overview
--------
Sming's host emulator allows easier debugging and development of embedded applications. This component named "HostEd" extends the host emulator
and facilitates testing functionality that only a real microcontroller can provide as for example digital I/O operations or SPI operations.

For example in order to run the Basic_Blink application under the host emulator and run the actual blinking of a LED on a microcontroller
we can compile the application using the following directives::

    make SMING_ARCH=Host ENABLE_HOSTED=tcp HOSTED_SERVER_IP=192.168.4.1

`SMING_ARCH=Host` instructs the build system to build the application for the Host architecture.
`ENABLE_HOSTED=tcp` instructs the host emulator to communication with the real microcontroller using TCP
`HOSTED_SERVER_IP=192.168.4.1` instructs the host emulator to connect to IP `192.168.4.1`.

We need to compile and flash also a special application on the desired microcontroller. That application will execute the commands send from the host emulator.
The compilation and flashing for ESP32, for example, can be done using the following commands::

    make hosted-app HOSTED_ARCH=Esp32 WIFI_SSID=YourSSID WIFI_PWD=YourPassword
    make hosted-flash

If you replace `HOSTED_ARCH=Esp32` with `HOSTED_ARCH=Esp8266` then the hosted application will be compiled and flashed on ESP8266 microcontroller.
Make sure to replace the values of  WIFI_SSID and WIFI_PWD with the actual name and password for the Access Point (AP).

Development
-----------
The communication between the host emulator and the hosted application is done via protocol buffer messages. The messages are describe in the `proto/hosted.proto`
file.

Adding a new command
~~~~~~~~~~~~~~~~~~~~
Adding a new command can be done in the following way.

1. You should add the command in the `proto/hosted.proto` file. Make sure to compile it using `nanopb`. This can be done with the commands below::

    cd $SMING_HOME/Components/Hosted/proto
    python $SMING_HOME/Components/nanopb/nanopb/generator/nanopb_generator.py hosted.proto

2. See `DigitalHosted.cpp` for inspiration how to add a new command to `Hosted-Lib`.
These functions will try to communicate with the hosted application on the microcontroller.

3. The actual work is done by the hosted application. See `app/application.cpp`. See the `init` method for inspiration how to register a new command that
should be executed on the controller.

.. envvar:: ENABLE_HOSTED

   Enables the hosted component. Valid values for the moment are:
   - tcp - for communication over TCP network.