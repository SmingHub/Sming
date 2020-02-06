modbusino RTU Library (modbus slave)
====================================

Configuration variables
-----------------------


.. envvar:: RS485_RE_PIN

   (Optional) Defines RE GPIO pin number. Defaults to 15.


.. envvar:: RS485_TX_LEVEL

   (Optional) Defines pin level (LOW or HIGH) when transmitting. Default HIGH.


.. envvar:: MB_PRE_TX_DELAY

   (Optional) Time to wait (in microseconds) after RE pin level change, before writing serial data. Used to ensure proper line detection by the master device. Defaults to 100.


.. envvar:: MB_POST_TX_DELAY

   (Optional) Delay (in microseconds) after data transmission, before RE pin level change. Defaults to 1000.
