Modbusino RTU generic sample
============================

.. highlight:: bash

The sample code provides three element uint16_t array used for the modbus slave registers. These values are printed each second through UART1.
mbSlaveLoopTimer checks for new data each 10 milliseconds (Only checks for new data. Serial communication is handled by hardware.)

Several environment variables (envvar:`RS485_RE_PIN` :envvar:`RS485_TX_LEVEL` :envvar:`MB_PRE_TX_DELAY` :envvar:`MB_POST_TX_DELAY`) can be used for configuration.
The slave address is defined with envvar:`MB_SLAVE_ADDR`. For example::

   make MB_SLAVE_ADDR=2

These variables can be listed with::

   make list-config
