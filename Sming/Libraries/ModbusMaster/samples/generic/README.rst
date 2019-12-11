ModbusMaster RTU generic sample
===============================

The generic sample calls mbLoop() each second. In mbLoop() the globalSeconds variable is incremented and sent to a slave device.
Then the same register address of the same slave device is read and the result is output via debugf() using UART1.

The modbus response timeout can be changed using :envvar:`MB_RESPONSE_TIMEOUT`.

Check out my `video <https://youtu.be/-RvoBn8fIBk>`__.