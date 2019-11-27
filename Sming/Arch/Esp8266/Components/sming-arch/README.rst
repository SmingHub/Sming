Sming (Esp8266)
===============

This Component builds a library containing architecture-specific code, and defines dependencies for Sming to build for the Esp8266.

SDK 3.0+
--------

Default: OFF. In order to use SDK 3.0.0 or newer please follow the instructions here :component-esp8266:`esp8266`.

Custom LWIP
-----------

`LWIP <https://savannah.nongnu.org/projects/lwip/>`__ (LightWeight IP) is a small independent implementation
of the TCP/IP protocol suite used widely in embedded systems. Sming supports several versions of this,
controlled by the :envvar:`ENABLE_CUSTOM_LWIP` setting.

.. envvar:: ENABLE_CUSTOM_LWIP

   0
      Use binary :component-esp8266:`esp-lwip` stack.
   1 (default)
      Use custom compiled :component-esp8266:`esp-open-lwip` stack.
      Compared with the Espressif stack, this uses less RAM but
      consumes FLASH (program) memory. All espconn\_* functions are turned off by default, so if you require these add
      the :envvar:`ENABLE_ESPCONN` =1 directive. The :sample:`Basic_SmartConfig` example sets this in its ``component.mk``
      file.
   2
      Use :component-esp8266:`lwip2` stack. This does not have support for espconn\_* functions.

.. envvar:: ENABLE_LWIP_DEBUG

   By default, some debug information will be printed for critical errors and situations.
   Set this to 1 to enable printing of all debug information.


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
