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
