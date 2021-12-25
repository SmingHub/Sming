Welcome to Sming Framework
==========================

.. highlight:: c++

**Let's do smart things!!!**

Sming is an asynchronous embedded C/C++ framework with superb performance and multiple network features.
Sming is open source, modular and supports :doc:`multiple architectures <features>` including ESP8266 and ESP32.

|samples|_

.. |samples| image:: combine.jpg
.. _samples: samples.html


Summary
-------
-  Superb performance and memory usage (Sming compiles to native firmware!)
-  Fast and user friendly development
-  Integrated :doc:`host emulator <arch/host/index>` to assist with developing, testing and debugging
   libraries and applications on a PC before uploading them to an actual microcontroller.
   Try it out online `here <https://www.katacoda.com/slaff/scenarios/sming-host-emulator>`__.
-  Built-in powerful wireless modules
-  Compatible with standard :doc:`libraries` - use popular hardware in few lines of code
-  Simple yet powerful hardware API wrappers

   -  Standard Arduino-style wrappers for simplicity and flexibility
   -  :library:`HardwareSPI` library provides advanced asynchronous SPI device master capability
      including use of ESP8266 'overlapped' mode for reduced pin usage and dual/quad I/O support.
   -  :library:`Graphics` implements asynchronous display control model with low memory usage,
      transparency (alpha-blending) and flexible scripted resource management.

-  Modular C++ installable file system interface

   -  Supports popular :library:`SPIFFS <Spiffs>` and :library:`LittleFS <LittleFS>` filing systems
   -  New :component:`FWFS <IFS>` read-only filesystem can be used as base file system,
      with read/write filesystems mounted in sub-directories
   -  Integrated metadata support (file times, security descriptors, user metadata)
   -  Access to Host (linux / Windows / MacOS) filing system via same API
   -  Integrated streaming archival system to support backups or file system compaction operations

-  Powerful asynchronous (async) :doc:`network stack </_inc/Sming/Components/Network/index>` including:

   -  Async TCP and UDP stack based on `LWIP <http://savannah.nongnu.org/projects/lwip/>`__.
   -  With clients supporting: HTTP, MQTT, WebSockets and SMTP.
   -  And servers for: DNS, FTP, HTTP(+ WebSockets), Telnet.
   -  With :doc:`SSL support <_inc/Sming/Components/ssl/index>` for all network clients and servers.
      Based on `axTLS <http://axtls.sourceforge.net/>`__ and `BearSSL <https://www.bearssl.org/>`__.
   -  Over-The-Air (OTA) firmware upgrades via HTTP(S) and MQTT(S).

-  :library:`Universal Plug and Play (UPnP) <UPnP>`
   framework supports both standard and custom devices with full ControlPoint support.

   -  Generates C++ code from standard :library:`UPnP XML schema <UPnP-Schema>`.
   -  Companion :library:`SSDP` library independently supports discovery on local network.
   -  :library:`GoogleCast` library allows control of ChromeCast devices or smart TVs supporting the GoogleCast protocol.
   -  :library:`HueEmulator` provides simple way to implement devices compatible with Amazon Alexa.

-  ESP8266 features

   -  Integrated boot loader :component:`rboot` with support for 1MB ROMs, OTA firmware updating and ROM switching.
   -  :doc:`Crash handlers <information/debugging>` for analyzing/handling system restarts due to fatal errors or WDT resets.
   -  :component-esp8266:`PWM support <driver>` based on `Stefan Bruens PWM <https://github.com/StefanBruens/ESP8266_new_pwm.git>`__.
   -  Optional :component-esp8266:`custom heap allocation <heap>` based on `Umm Malloc <https://github.com/rhempel/umm_malloc.git>`__.
   -  Based on :component-esp8266:`Espressif NONOS SDK Version 3 <esp8266>`.

-  ESP32 features

   - Based on :component-esp32:`ESP IDF SDK <esp32>`


.. toctree::
   :caption: Contents:
   :hidden:

   getting-started/index
   features
   framework/index
   samples
   libraries
   api/index
   information/index
   upgrading/index
   experimental/index
   contribute/index
   tools/index
   troubleshooting/index
   about

Indices and tables
------------------

* :ref:`genindex`
