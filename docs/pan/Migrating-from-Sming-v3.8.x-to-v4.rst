Summary
=======

With the new Sming version 3.9.0 there will be a lot of backwards
incompatible changes. This page is provided to help with migrating your
applications.

Header files
============

The folder structure has been revised to provide support for additional
architectures, such as the ESP32.

The ``Sming/Arch`` directory should never be accessed directly: it
contains specific files for the target architecture, and may provide
different header file versions to the main ones. Instead, consider these
directories to be your ‘root’ include directories:

::

   Sming
   Sming/Components
   Sming/Core
   Sming/Wiring

Examples of #include statements:

====================================================     =====================================
Old-style                                                Recommended
====================================================     =====================================
``"SmingCore/SmingCore.h"``                              ``<SmingCore.h>``
``"SmingCore/Network/SmtpClient.h"``                     ``<Network/SmtpClient.h>``
``"SmingCore/Data/Stream/FlashMemoryStream.h"``          ``<Data/Stream/FlashMemoryStream.h>``
``"Wiring/WString.h"``                                   ``<WString.h>``
``"SmingCore/Platform/Station.h"``                       ``<Platform/Station.h>``
====================================================     =====================================

Changed Headers
---------------

If you use some of the includes below directly in your application make
sure to apply the following changes:

================== ================================= =====================
Description        Old name                          New name
================== ================================= =====================
uart driver        ``"espinc/uart.h"``               ``<driver/uart.h>``
flesh memory       ``"flashmem.h"``                  ``<esp_spi_flash.h>``
C compatible types ``<espinc/c_types_compatible.h>`` ``<c_types.h>``.
================== ================================= =====================

user_include.h
--------------

This file is generally #included ahead of everything else so that common
architecture-specific definitions are available. Unless you’ve made
changes to the file, it is not required and you should delete it: Sming
provides a default which will be used.

If you have made customisations, please amend the file as follows:

::

   #pragma once
   #include_next <user_config.h>

   << User Customisations here >>

Application Makefile
====================

Change

::

   include $(SMING_HOME)/Makefile-rboot.mk

to

::

   include $(SMING_HOME)/Makefile-app.mk  

JSON
====

ArduinoJson has been updated to Version 6 from version 5. See the
`Version 6 Migration Guide <https://arduinojson.org/v6/doc/upgrade>`__
for details.

Also note some methods of ``JsonVariant`` have been removed,
replacements are:

``asString()`` -> ``as<char*>()`` or ``as<const char*>``. Note that
``as<String>`` produces a serialized version, so you’ll get “null”
instead of an empty/invalid result String.

``asArray()`` -> ``as<JsonArray>()``

``asObject()`` -> ``as<JsonObject>()``
