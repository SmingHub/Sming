Esptool
=======

This Component provides Espressif's tool for reading and writing firmware and other data to hardware.

Options
-------

.. envvar:: SPI_SPEED

   [read-only]  Set by :ref:`hardware_config`.

   Clock speed for flash memory (20, 26, 40 or 80). Default is 40.


.. envvar:: SPI_MODE

   [read-only]  Set by :ref:`hardware_config`.

   Flash memory operating mode (quot, dio, dout, qio). Default is qio.

.. envvar:: SPI_SIZE

   [read-only]  Set by :ref:`hardware_config`.

   Size of flash memory chip (256KB, 512KB, 1MB, 2MB, 4MB). Default is 512K bytes.

   The default hardware profile ``standard`` sets this to 1MB.
   You can set ``HWCONFIG=standard-4m`` to increase it or create a custom :ref:`hardware_config` for your project.


.. envvar:: ESPTOOL

   Full path of esptool.py


.. envvar:: COM_PORT_ESPTOOL

   Port to use for flashing device. Default is :envvar:`COM_PORT`.


.. envvar:: COM_SPEED_ESPTOOL

   Baud rate to use for flashing device. Default is :envvar:`COM_SPEED`.
