HostEd
============

.. highlight:: bash

The hosted component allows Sming's host emulator to run parts of the code on an actual microcontroller.
TBD...

Configuration variables
-----------------------

Serial Communications
~~~~~~~~~~~~~~~~~~~~~

.. envvar:: COM_SPEED

   Default baud rate for serial port.

   This will recompile your application to use the revised baud rate.
   Note that this will change the default speed used for both flashing and serial comms.
   See also :component-esp8266:`esptool` and :component:`terminal` for further details.


Development
-----------
If you are interested in ...
TDB

Protobuf with nanopb::

    cd $SMING_HOME/Components/Hosted/proto
    python ~/dev/nanopb/generator/nanopb_generator.py hosted.proto

TBD...