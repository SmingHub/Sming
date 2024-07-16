Welcome to Sming Framework
==========================

.. highlight:: c++

**Let's do smart things!!!**

Sming is an asynchronous embedded C++ framework with superb performance and multiple network features.
Sming is open source, modular and supports :doc:`multiple architectures <features>`:
:doc:`ESP8266 </_inc/Sming/Arch/Esp8266/README>`,
:doc:`ESP32 </_inc/Sming/Arch/Esp32/README>`,
:doc:`RP2040 </_inc/Sming/Arch/Rp2040/README>`,
and :doc:`Host Emulation </_inc/Sming/Arch/Host/README>`.

|samples|_

.. |samples| image:: combine.jpg
.. _samples: samples.html


Summary
-------
-  Superb performance and memory usage: Sming compiles to native firmware!
-  Fast and user-friendly C++ development
-  Integrated :doc:`host emulator <arch/host/index>` to assist with developing, testing and debugging
   libraries and applications on a PC. It even includes networking support!
   Try it out online `here <https://killercoda.com/slaff/scenario/sming-host-emulator>`__.
-  Built-in powerful wireless modules
-  Compatible with standard :doc:`libraries` - use popular hardware in few lines of code
-  Integrated :component:`FlashString` support for direct use of structured data stored in flash memory.
   Features :c:macro:`IMPORT_FSTR` macro for linking external binary files: no conversion to C arrays required!
-  Simple yet powerful hardware API wrappers

   -  Standard Arduino-style wrappers for simplicity and flexibility
   -  :library:`HardwareSPI` library provides advanced asynchronous SPI device master capability
      including use of ESP8266 'overlapped' mode for reduced pin usage and dual/quad I/O support.
   -  :library:`Graphics` implements asynchronous display control model with low memory usage,
      transparency (alpha-blending), flexible scripted resource management and graphical layout editor.

-  Modular C++ Storage and Installable File System interfaces

   -  Supports popular :library:`SPIFFS <Spiffs>` and :library:`LittleFS <LittleFS>` filing systems
   -  New :component:`FWFS <IFS>` read-only high-performance filesystem can be used as base file system,
      with read/write filesystems mounted in sub-directories
   -  Integrated metadata support (file times, security descriptors, user metadata)
   -  Access to Host (GNU/Linux, Windows, MacOS) filing system via same API
   -  Integrated streaming archival system to support backups or file system compaction operations
   -  Modular :component:`Storage` layer providing consistent access to off-device media such
      as :library:`SD Cards <SdStorage>` or :library:`USB mass storage <USB>` for devices with USB support.

-  Powerful asynchronous (async) :doc:`network stack </_inc/Sming/Components/Network/index>` including:

   -  Async TCP and UDP stack based on `LWIP <http://savannah.nongnu.org/projects/lwip/>`__.
   -  With clients supporting: HTTP, MQTT, WebSockets and SMTP.
   -  And servers for: DNS, FTP, HTTP(+ WebSockets), Telnet.
   -  Optional :doc:`SSL support <_inc/Sming/Components/ssl/index>` support for network clients and servers.
      Choose either `axTLS <http://axtls.sourceforge.net/>`__ or `BearSSL <https://www.bearssl.org/>`__.
   -  Over-The-Air (OTA) firmware upgrades via HTTP(S) and MQTT(S).

-  :library:`Universal Plug and Play (UPnP) <UPnP>`
   framework supports both standard and custom devices with full ControlPoint support.

   -  Generates C++ code from standard :library:`UPnP XML schema <UPnP-Schema>`.
   -  Companion :library:`SSDP` library independently supports discovery on local network.
   -  :library:`GoogleCast` library allows control of ChromeCast devices or smart TVs supporting the GoogleCast protocol.
   -  :library:`HueEmulator` provides simple way to implement devices compatible with Amazon Alexa.


.. toctree::
   :caption: Contents:
   :hidden:

   getting-started/index
   tools/index
   debugging/index
   features
   framework/index
   samples
   libraries
   api/index
   information/index
   upgrading/index
   experimental/index
   contribute/index
   troubleshooting/index
   about

Indices and tables
------------------

* :ref:`genindex`
