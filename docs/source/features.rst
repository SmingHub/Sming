Features
========

There are multiple custom features that can be enabled by default. For
example: SSL support, custom LWIP, open PWM, custom heap allocation,
more verbose debugging, etc.

-  **Custom LWIP**: (default: ON) By default we are using custom
   compiled LWIP stack instead of the binary one provided from
   Espressif. This increases the free memory and decreases the space on
   the flash. All espconn_\* functions are turned off by default. If
   your application requires the use of some of the espconn_\* functions
   then add the ENABLE_ESPCONN=1 directive. See ``Makefile-user.mk``
   from the
   `Basic_SmartConfig <https://github.com/SmingHub/Sming/blob/develop/samples/Basic_SmartConfig/Makefile-user.mk#L41>`__
   application for examples. If you would like to use the binary LWIP
   then you should turn off the custom LWIP compilation by providing
   ``ENABLE_CUSTOM_LWIP=0``.
-  **LWIP 2**: (default: OFF) LWIP 2 stands for LightWeight IP stack,
   version 2. In order to enable that feature you should (re)compile the
   Sming library AND your application using the following directive
   ENABLE_CUSTOM_LWIP=2. LWIP v2 does not have support for espconn_\*
   functions. This feature is still **experimental** which means that we
   do not recommend it in production.
-  **SSL support**: (default: OFF) The SSL support is not built-in by
   default to conserve resources. If you want to enable it then take a
   look at the
   `Readme <https://github.com/SmingHub/Sming/blob/develop/samples/Basic_Ssl/README.md>`__
   in the Basic_Ssl samples.
-  **Custom PWM**: (default: ON) If you don’t want to use the `open PWM
   implementation <https://github.com/StefanBruens/ESP8266_new_pwm>`__
   then compile your application with ``ENABLE_CUSTOM_PWM=0``. There is
   no need to recompile the Sming library.
-  WPS: (default: OFF) The WPS support (Wi-Fi Protected Setup) is not
   activated by default to preserve resources. To enable WPS, use the
   switch ENABLE_WPS=1 for compiling Sming.
-  **Custom serial baud rate**: (default: OFF) The default serial baud
   rate is 115200. If you want to change it to a different baud rate you
   can recompile Sming and your application changing the
   ``COM_SPEED_SERIAL`` directive. For example
   ``COM_SPEED_SERIAL=921600``.
-  **Custom heap allocation**: (default: OFF) If your application is
   experiencing heap fragmentation then you can try the
   `umm_malloc <https://github.com/rhempel/umm_malloc>`__ heap
   allocation. To enable it compile Sming with ``ENABLE_CUSTOM_HEAP=1``.
   In order to use it in your sample/application make sure to compile
   the sample with ``ENABLE_CUSTOM_HEAP=1``. **Do not enable custom heap
   allocation and -mforce-l32 compiler flag at the same time**.
-  **Debug information log level and format**: There are four debug
   levels: debug=3, info=2, warn=1, error=0. Using
   ``DEBUG_VERBOSE_LEVEL`` you can set the desired level (0-3). For
   example ``DEBUG_VERBOSE_LEVEL=2`` will show only info messages and
   above. Another make directive is ``DEBUG_PRINT_FILENAME_AND_LINE=1``
   which enables printing the filename and line number of every debug
   line. This will require extra space on flash. Note: you can compile
   the Sming library with a set of debug directives and your project
   with another settings, this way you can control debugging separately
   for Sming and your application code.
-  **Debug information for custom LWIP**: If you use custom LWIP (see
   above) some debug information will be printed for critical errors and
   situations. You can enable all debug information printing using
   ``ENABLE_LWIPDEBUG=1``. To increase debugging for certain areas you
   can modify debug options in
   ``third-party/esp-open-lwip/include/lwipopts.h``.
-  **Interactive debugging on the device**: (default: OFF) In order to
   be able to debug live directly on the ESP8266 microcontroller you
   should re-compile the Sming library and your application with
   ``ENABLE_GDB=1`` directive. See
   `LiveDebug <https://github.com/SmingHub/Sming/tree/develop/samples/LiveDebug>`__
   sample for more details.
-  **CommandExecutor feature**: (default: ON) This feature enables
   execution of certain commands by registering token handlers for text
   received via serial, websocket or telnet connection. If this feature
   is not used additional RAM/Flash can be obtained by setting
   ``ENABLE_CMD_EXECUTOR=0``. This will save ~1KB RAM and ~3KB of flash
   memory.
-  **SDK 3.0+**: (default: OFF) In order to use SDK 3.0.0 or newer you
   should set one environment variable before (re)compiling Sming AND
   applications based on it. The variable is SDK_BASE and it should
   point to ``$SMING_HOME/third-party/ESP8266_NONOS_SDK``.

For Windows you need to do:

::

   set SDK_BASE %SMING_HOME%//third-party/ESP8266_NONOS_SDK

For Linux(bash) you need to do:

::

   export SDK_BASE="$SMING_HOME/third-party/ESP8266_NONOS_SDK"

Read the comments from `this
URL <https://github.com/SmingHub/Sming/pull/1264>`__ for known issues.
This feature is still **experimental** which means that we still do not
recommend it in production.
