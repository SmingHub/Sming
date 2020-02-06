Basic Serial
============

.. highlight:: bash

Demonstrates Sming's asynchronous capabilities using the UART to send and receive simultaneously
using two serial ports.

Use the primary serial port to enter commands:

cat
   Send the contents of the *Readme.md* file to the second serial port.

text
   Echo a block of text

Note that you can continue to type commands while serial data is being transmitted
as all operations are fully asynchronous. This becomes more obvious if you try using
a low baud rate, for example::

   make COM_SPEED_SERIAL=9600 COM_SPEED_ESPTOOL=921600

We'd still like a decent speed for flashing though!

If you want to set the two ports to different speeds the code will need to be modified.
