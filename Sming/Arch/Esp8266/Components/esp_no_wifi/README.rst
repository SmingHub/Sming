Esp8266 No WiFi
===============

.. highlight:: bash

Background
----------

Based on SDK 3.0.1 disassemblies with reference and thanks to:

-  https://github.com/pvvx/esp8266web Further development of

   -  https://github.com/pvvx/SDKnoWiFi
   -  https://github.com/pvvx/MinEspSDKLib

-  https://github.com/cnlohr/nosdk8266

The problem we have is to disentangle the wifi functions from the SDK within the
``app_main.o`` and ``user_interface.o`` modules from ``libmain.a``.
In particular, the :c:func:`user_start` code which mixes up
wifi stuff with system code so it cannot be easily separated.

The SDKnoWiFi implements the startup code and some system functions but contains a lot of stuff
which is provided by the SDK and in other parts of the Sming framework. We need to provide
replacement functions to interoperate correctly with the remaining SDK code.

Process
-------

This is the approach used to create this Component.

1. Remove user_interface from libmain::

      ar d libmain.a user_interface.o

2. Implement ``call_user_start_local``. The actual entry point is ``call_user_start``,
   in vector.o, which is required. (Note: This symbol was -u in the linker script but this
   is un-necessary and has been removed.)

3. Build and implement missing functions.

Where functions can be passed directly to the ROM code (e.g. system_os_task -> ets_task)
these are defined in the no.wifi.ld linker script.

Other code is located in a .c file named according to the module it replaces in libmain.a.

There are various options for integrating this into Sming, but I've decided the most useful
way is to have it as a link-time option. We either link ``esp_wifi`` or ``esp_no_wifi``
as required, no rebuild to the framework is necessary. This is controlled by the
:c:macro:`DISABLE_WIFI` setting.

This means that if there are any WiFi routines used in the project the link step will fail.

An alternate approach is to separate out the Network/WiFi stuff in the framework into
separate Components which are themselves only included. However, some of the network code
could be useful in a non-networked application. There may even be an alternate physical
network layer implemented (e.g. Ethernet over SPI) so it's simplest to just leave the
framework intact.


Library Disassembly
-------------------

You can create a disassembly of the relevant SDK libraries for inspection by running this
command from a project directory::

   make sdk-disassemble

If you want to disassemble other SDK libraries, do this::

   make sdk-disassemble SDK_LIBLIST="crypto net80211"

