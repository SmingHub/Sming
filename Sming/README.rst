Sming (main)
============

.. highlight:: bash

This is the main Sming Component containing all architecture-independent code.
All architecture-specific stuff is in either :component-esp8266:`sming-arch` or :component-host:`sming-arch`.

Serial baud rate
----------------

.. envvar:: COM_SPEED

   Default baud rate for serial port.

   This will recompile your application to use the revised baud rate.
   Note that this will change the default speed used for both flashing and serial comms.
   See also :component-esp8266:`esptool` and :component:`terminal` for further details.

The default rate for serial ports is 115200 baud. You can change it like this:

::

   make COM_SPEED=921600


Debug information log level and format
--------------------------------------

.. envvar:: DEBUG_VERBOSE_LEVEL

   When compiled in debug mode (:envvar:SMING_RELEASE undefined) there are four debug levels in increasing level of verbosity:

   * 0: errors
   * 1: warnings
   * 2: information (default)
   * 3: debug

Change it like this:

::

   make DEBUG_VERBOSE_LEVEL=3


.. envvar:: DEBUG_PRINT_FILENAME_AND_LINE

   Set this to 1 to include the filename and line number in every line of debug output.
   This will require extra space on flash.

.. note::
   If you change these settings and want them applied to Sming, not just your project, then you'll
   need to recompile all components like this:

   ::
   
      make components-clean
      make DEBUG_VERBOSE_LEVEL=3


Release builds
--------------

.. envvar:: SMING_RELEASE

   By default, this value is undefined to produce a build with debug output.
   To build for release, do this:

   ::
   
      make SMING_RELEASE=1

   This remains in force until you change it back:
   
   ::
   
      make SMING_RELEASE=


Command Executor
----------------

.. envvar:: ENABLE_CMD_EXECUTOR

   Default: ON. This feature enables
   execution of certain commands by registering token handlers for text
   received via serial, websocket or telnet connection. If this feature
   is not used additional RAM/Flash can be obtained by setting
   ``ENABLE_CMD_EXECUTOR=0``. This will save ~1KB RAM and ~3KB of flash
   memory.

.. doxygengroup:: commandhandler
 

WiFi Connection
---------------

If you want to provide a default SSID and Password for connection to your default Access Point, you can do this:

::

   make WIFI_SSID=MyAccessPoint WIFI_PWD=secret

These are provided as #defined symbols for your application to use. See :sample:`Basic_WiFi` for a simple example,
or :sample:`MeteoControl` for a more flexible solution using configuration files.

.. envvar:: WIFI_SSID

   SSID identifying default Access Point to connect to. By default, this is undefined.


.. envvar:: WIFI_PWD

   Password for the :envvar:`WIFI_SSID` Access Point, if required. If the AP is open then
   leave this undefined.


Localisation
------------

.. envvar:: LOCALE

   Sming can format dates/time values based on a country code identified by this value.
   This is provided as a #define symbol for your application to use.
   See :source:`Sming/Core/SmingLocale.h` for further details.


Components
----------

 .. toctree::
   :glob:
   :maxdepth: 1
 
   Components/*/index
