Esp8266 Open LWIP (version 1)
=============================

This Component provides the default Sming LWIP stack, which uses less RAM than the Espressif version.

See :envvar:`ENABLE_CUSTOM_LWIP`.

.. envvar:: ENABLE_LWIPDEBUG

   0 (default)
      Standard build
   1
      Enable debugging output
   
   You can increase debugging for certain areas by modifying debug options in ``esp-open-lwip/include/lwipopts.h``.

.. envvar:: ENABLE_ESPCONN

   The Espressif SDK defines a network API consisting of functions which start with ``espconn_``.

   0 (default)
      Disabled

   1
      Enable espconn\_ functions
