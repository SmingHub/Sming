modbusino RTU Library (modbus slave)
====================================

modbusino is lightweight RTU `Modbus <https://en.wikipedia.org/wiki/Modbus>`__
slave library that supports 'read holding registers' and 'write multiple registers' functions.
Please note that prior to commit 02dff3c (branch Sming, port URL https://github.com/kmihaylov/modbusino)
a delay may occur after sending a message (more information can be found in the PR thread #2043,
https://github.com/SmingHub/Sming/pull/2043#issuecomment-615945823).


Configuration variables
-----------------------


.. envvar:: RS485_RE_PIN

   Default: 15

   GPIO pin number for RE (Receive-Enable) output.


.. envvar:: RS485_TX_LEVEL

   Default: HIGH.

   Active level for RE pin during transmission.
