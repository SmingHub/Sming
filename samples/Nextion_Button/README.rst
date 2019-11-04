Nextion Button
==============

See this example in action: https://youtu.be/lHk6fqDBHyI

The HMI file included in this example needs to be compiled with the
Nextion Editor and uploaded to the Nextion display using standard method.

On WEMOS mini D1 (where this example was tested), the Nextion device is
connected to RX/TX pins as required.
BUT it needs to be disconnected when uploading the firmware.

So the process is:

1. Make changes to the cpp code
2. Build it using "make"
3. Disconnect the Nextion display if it is connected to Rx/Tx.
4. Upload firmware (built in step 2) using "make flash".
5. Connect Nextion display back again to Rx/Tx.

.. note::

   Always unplug the ESP8266 from USB (connecting with computer) or disconnect power
   before fiddling with the connections between ESP8266 and Nextion display.
