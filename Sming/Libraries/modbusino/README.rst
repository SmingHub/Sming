modbusino RTU Library (modbus slave)
====================================

The author's project is at https://github.com/stephane/modbusino

modbusino is lightweight RTU modbus slave library that supports 'read holding registers' and 'write multiple registers' functions.

Four environment variables define the RS485 interface hardware pins and pre- and post-transmission delay.

.. envvar:: RS485_RE_PIN

   :envvar:`RS485_RE_PIN` defines the RE GPIO pin number.


.. envvar:: RS485_TX_LEVEL

   :envvar:`RS485_TX_LEVEL` defines the pin level (LOW or HIGH) when transmitting.


.. envvar:: MB_PRE_TX_DELAY

   With :envvar:`MB_PRE_TX_DELAY` (in microseconds) the delay between changing the RE pin level and sending data can be set. Used to ensure proper line detection by the master device.


.. envvar:: MB_POST_TX_DELAY

   :envvar:`MB_POST_TX_DELAY` is the time in microseconds between the end of data transmission and releasing the line.
