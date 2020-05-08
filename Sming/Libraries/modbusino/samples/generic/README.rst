Modbusino RTU generic sample
============================

.. highlight:: bash

The sample code provides three element uint16_t array used for the modbus slave registers. These values are printed each second through UART1.
mbPrint() is triggered on valid write multiple registers (0x10) command.

Several environment variables (envvar:`RS485_RE_PIN` :envvar:`RS485_TX_LEVEL`) can be used for configuration.
The slave address is defined with envvar:`MB_SLAVE_ADDR`. For example::

   make MB_SLAVE_ADDR=2

These variables can be listed with::

   make list-config
