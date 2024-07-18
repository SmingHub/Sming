UART: Universal Asynchronous Receive/Transmit
=============================================

Custom asynchronous serial port driver.

.. c:macro:: UART_ID_SERIAL_USB_JTAG

   Some SOC variants, such as esp32c3, have an integrated USB JTAG/Serial port.
   If so, then this value defines the associated serial port number.

   Requires ESP-IDF version 5.2 or later.

   Example usage::

      #ifdef UART_ID_SERIAL_USB_JTAG
      Serial.setPort(UART_ID_SERIAL_USB_JTAG);
      #endif
      Serial.begin(...);


.. doxygengroup:: uart_driver
   :content-only:
   :members:
