Sming (Esp32)
===============

This Component builds a library containing architecture-specific code, and defines dependencies for Sming to build for the Esp32.

Interactive debugging on the device
-----------------------------------

.. envvar:: ENABLE_GDB

   In order to be able to debug live directly on the ESP8266 microcontroller you
   should re-compile your application with ``ENABLE_GDB=1`` directive.

   undefined (default)
      Compile normally
   1
      Compile with debugging support provided by :component-esp8266:`gdbstub`.
      See also the :sample:`LiveDebug` sample.
