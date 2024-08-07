Sming (Esp8266)
===============

This Component builds a library containing architecture-specific code, and defines dependencies for Sming to build for the Esp8266.

No-WiFi build
-------------

.. note::

   This is an EXPERIMENTAL feature. Not all hardware functions may be available.

If a project does not require WiFi (or networking) then setting the :envvar:`DISABLE_WIFI` variable
will reduce code size and RAM usage significantly.
It does this using an un-official :component-esp8266:`esp_no_wifi` Component.



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
      consumes FLASH (program) memory. All ``espconn_*`` functions are turned off by default, so if you require these add
      the :envvar:`ENABLE_ESPCONN=1 <ENABLE_ESPCONN>` directive.
      The :sample:`Basic_SmartConfig` example sets this in its ``component.mk`` file.
   2
      Use :component-esp8266:`lwip2` stack. This does not have support for ``espconn_*`` functions.

.. envvar:: ENABLE_LWIP_DEBUG

   By default, some debug information will be printed for critical errors and situations.
   Set this to 1 to enable printing of all debug information.
