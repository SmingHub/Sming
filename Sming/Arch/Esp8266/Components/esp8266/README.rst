Esp8266 Core Component
======================

.. highlight:: bash

Contains startup code, crash handling and additional Esp8266-specific
support code. Sming may be built using a pre-installed SDK, or by using
the current version 3 SDK as a submodule.

.. attention::

   At time of writing, SDK 3 does not appear to support use of devices with 256K or 512K memory,
   such as the ESP-01. For now, please use the default SDK 1.5.4 or SDK 2.0.0.

.. envvar:: SDK_BASE

   Points to the location of the Espressif Non-OS SDK. To use the Espressif version 3 SDK, you need
   only set this variable to point at the Sming repository (:envvar:`SMING_HOME`). The actual location
   will be subsituted by the build system and the SDK pulled in via GIT.

   So for Windows you need to do:

   .. code-block:: batch

      set SDK_BASE=%SMING_HOME%

   For Linux (bash):

   ::

      export SDK_BASE="$SMING_HOME"

   If you change this value then your application and Sming must both be recompiled:

   ::
   
      make components-clean clean
      make

.. envvar:: SDK_INTERNAL

   **READONLY** When compiled using the current (version 3+) Espressif SDK this value is set to 1.

.. envvar:: SDK_LIBDIR

   **READONLY** Path to the directory containing SDK archive libraries

.. envvar:: SDK_INCDIR

   **READONLY** Path to the directory containing SDK header files
