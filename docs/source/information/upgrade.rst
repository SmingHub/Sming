**********************
Upgrading to Sming 4.0
**********************

Summary
=======

With Sming version 4.0 there are some backwards
incompatible changes. This page is provided to help with migrating your
applications.

You can find more detailed information in the
:doc:`/_inc/Sming/building` readme.

Header files
============

The folder structure has been revised to provide support for additional
architectures, such as the Host Emulator, and in the future the ESP32.

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
C compatible types ``<espinc/c_types_compatible.h>`` ``<c_types.h>``
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


If you're using ``#include <SmingCore.h>`` then you don't need
``#include <user_config.h>`` as this is included automatically.

Application Makefile
====================

* Rename ``Makefile-user.mk`` file to ``component.mk``.
* Replace the file ``Makefile`` with the one from the ``Basic_Blink``
  sample project. If you've ignored the instructions and modified the
  file (!) then you'll need to move those changes into your new
  ``component.mk`` file instead.
* Sming uses the ``#pragma once`` statement for header guards, so
  consider updating your own files if you're not already doing this.

Arduino Libraries
=================

Your project must specify which Arduino Libraries it uses (if any). Do
this by setting ``ARDUINO_LIBRARIES`` in your project's
``component.mk`` file. For example:

.. code-block:: make

   ARDUINO_LIBRARIES := OneWire

This change means only the libraries you require for a project need to
be built.


JSON
====

ArduinoJson is now an optional Component, so you need to make a couple
of changes to use it:

* Add ``ArduinoJson6`` to the ``ARDUINO_LIBRARIES`` variable in your
  project's ``component.mk`` file. (e.g.
  ``ARDUINO_LIBRARIES = ArduinoJson6``) To support migration of
  existing projects, you can elect to continue using version 5 by
  specifying ``ArduinoJson5`` instead.
* Add ``#include <JsonObjectStream.h>`` to your source code. If you're
  not using the stream class, add ``#include <ArduinoJson.h>`` instead.

See library documentation for further details:

* :library:`ArduinoJson6`
* :library:`ArduinoJson5`
