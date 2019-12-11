Sming (main)
============

.. highlight:: bash

This is the main Sming Component containing all architecture-independent code.
All architecture-specific stuff is in either :component-esp8266:`sming-arch` or :component-host:`sming-arch`.

Configuration variables
-----------------------

Serial Communications
~~~~~~~~~~~~~~~~~~~~~

.. envvar:: COM_SPEED

   Default baud rate for serial port.

   This will recompile your application to use the revised baud rate.
   Note that this will change the default speed used for both flashing and serial comms.
   See also :component-esp8266:`esptool` and :component:`terminal` for further details.

The default rate for serial ports is 115200 baud. You can change it like this:

::

   make COM_SPEED=921600


Debug information log level and format
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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


Task Queue
----------

The task queue is used for *System.queueCallback()* calls.

.. envvar:: TASK_QUEUE_LENGTH

   Maximum number of entries in the task queue (default 16). Must be a power of 2.


.. envvar:: ENABLE_TASK_COUNT

   If problems are suspected with task queuing, it may be getting flooded.
   For this reason you should check the return value from `queueCallback()`.
   
   You can enable this option to keep track of the number of active tasks,
   *System::getTaskCount()*, and the maximum, *System::getMaxTaskCount()*.

   By default this is disabled and both methods will return 255.
   This is because interrupts must be disabled to ensure an accurate count,
   which may not be desirable.


String Optimisation
-------------------

The ``String`` class is probably the most used class in the Arduino world.
It is also heavily used within Sming.

Unfortunately it gets the blame for one of the most indidious problems in the
embedded world, `heap fragmentation <https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html>`__.

To alleviate this problem, Sming uses a technique known as *Small String Optimisation*,
which uses the available space inside the String object itself to avoid using the heap for small allocations
of 10 characters or fewer.

This was lifted from the `Arduino Esp8266 core <https://github.com/esp8266/arduino/pull/5690>`.
Superb work - thank you!

We've also added an experimental feature which lets you increase the size of a String object to
reduce heap allocations further. The effect of this will vary depending on your application,
but you can see some example figures in :pull-request:`1951`.

Benefits of increasing STRING_OBJECT_SIZE:

-  Increase code speed
-  Fewer heap allocations

Drawbacks:

-  Increased static memory usage for global/static String objects or embedded within global/static class instances.
-  A String can use SSO _or_ the heap, but not both together, so when/if it switches to heap mode
   then any additional space will remain unused, even if the String is itself allocated on the heap.


.. envvar:: STRING_OBJECT_SIZE

   minimum: 12 bytes (default)
   maximum: 128 bytes
   
   Must be an integer multiple of 4 bytes.

   Allows the size of a String object to be changed to increase the string length available
   before the heap is used.

   .. note::

      The current implementation uses one byte for a NUL terminator, and another to store the length,
      so the maximum SSO string length is (STRING_OBJECT_SIZE - 2) characters.

      However, the implementation may change so if you need to check the maximum SSO string size
      in your code, please use ``String::SSO_CAPACITY``.


Release builds
~~~~~~~~~~~~~~

.. envvar:: SMING_RELEASE

   By default, this value is undefined to produce a build with debug output.
   To build for release, do this:

   ::
   
      make SMING_RELEASE=1

   This remains in force until you change it back:
   
   ::
   
      make SMING_RELEASE=


WiFi Connection
---------------

.. envvar:: ENABLE_WPS

   Set to 1 to enable WiFi Protected Setup (WPS)
   WPS is not enabled by default to preserve resources, and because there may be security implications which you should consider carefully.

.. envvar:: ENABLE_SMART_CONFIG

   Set to 1 to enable WiFi Smart Configuration API
   SmartConfig requires extra libraries and :envvar:`ENABLE_ESPCONN`.
   See :sample:`Basic_SmartConfig` sample application.

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
~~~~~~~~~~~~

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
