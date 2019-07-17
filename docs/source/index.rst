Welcome to Sming's documentation
================================

.. highlight:: c++

**Let's do smart things!!!**

Sming is an Open Source framework for high efficiency WiFi SoC ESP8266
native development with C++ language.

.. image:: combine.png
   :target: https://github.com/SmingHub/Sming/wiki/examples


Summary
-------

-  Highly effective in performance and memory usage (this is native firmware!)
-  Simple and powerful hardware API wrappers
-  Fast & user friendly development
-  Work with GPIO in Arduino style
-  Compatible with standard :doc:`Arduino libraries <libraries/index>` - use any popular hardware in few lines of code
-  Integrated boot loader :component-esp8266:`rboot`
   with support for 1MB ROMs, OTA firmware updating and ROM switching
-  Built-in file system: :component:`spiffs`
-  Built-in powerful wireless modules
-  Powerful asynchronous (async) network stack

   -  Async TCP and UDP stack based on `LWIP <http://savannah.nongnu.org/projects/lwip/>`__
   -  With clients supporting: HTTP, MQTT, WebSockets and SMTP
   -  And servers for: DNS, FTP, HTTP(+ WebSockets), Telnet
   -  With SSL support for all network clients and servers based on
      `axTLS 2.1+ <https://github.com/igrr/axtls-8266>`__ with
      `Lwirax <https://github.com/attachix/lwirax/>`__.
   -  Out of the box support for OTA over HTTPS.

-  :doc:`Crash handlers <information/debugging>` for analyzing/handling
   system restarts due to fatal errors or WDT resets.
-  :component-esp8266:`PWM support <pwm_open>` based on `Stefan Bruens
   PWM <https://github.com/StefanBruens/ESP8266_new_pwm.git>`__
-  Optional :component-esp8266:`custom heap allocation <custom_heap>`
   based on `Umm Malloc <https://github.com/rhempel/umm_malloc.git>`__
-  Based on :component-esp8266:`Espressif NONOS SDK <esp8266>`. Tested with versions 1.5, 2.0 and 3.0.


Getting Started
---------------

See the :doc:`Quick-start <quick-start/index>` page for your respective operating system.


.. toctree::
   :caption: Contents:
   :hidden:

   quick-start/index
   host-emulator
   features
   libraries/index
   api/index
   information/index
   experimental/index
   contribute/index
   tools/index
   troubleshooting/index

Indices and tables
------------------

* :ref:`genindex`
