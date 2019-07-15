Esp8266 Core Component
======================

Contains startup code, crash handling and additional Esp8266-specific
support code. Sming may be built using a pre-installed SDK, or by using
the current version 3 SDK as a submodule.

.. envvar:: SDK_BASE

   Points to the location of the Espressif Non-OS SDK. To use the Espressif version 3 SDK, you need
   only set this variable to point at the Sming repository (:envvar:`SMING_HOME`). The actual location
   will be subsituted by the build system and the SDK pulled in via GIT.

   So for Windows you need to do:

   ::
   
      set SDK_BASE=%SMING_HOME%

   For Linux (bash):
   
   ::
   
      export SDK_BASE="$SMING_HOME"

   If you change this value then your application and Sming must both be recompiled:
   
   ::
   
      make components-clean clean
      make
   
