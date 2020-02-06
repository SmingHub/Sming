modbusino RTU Library (modbus slave)
====================================

modbusino is lightweight RTU `Modbus <https://en.wikipedia.org/wiki/Modbus>`__
slave library that supports 'read holding registers' and 'write multiple registers' functions.


Configuration variables
-----------------------


.. envvar:: RS485_RE_PIN

   Default: 15

   GPIO pin number for RE (Receive-Enable) output.


.. envvar:: RS485_TX_LEVEL

   Default: HIGH.

   Active level for RE pin during transmission.


.. envvar:: MB_PRE_TX_DELAY

   Default: 100 (microseconds).

   Time to wait after RE pin level change, before writing serial data. Used to ensure proper line detection by the master device.


.. envvar:: MB_POST_TX_DELAY

   Default: 1000 (microseconds).

   Delay after data transmission, before RE pin level change.
