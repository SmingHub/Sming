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
   See also :component:`esptool` and :component:`terminal` for further details.

The default rate for serial ports is 115200 baud. You can change it like this::

   make COM_SPEED=921600


Debug information log level and format
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. envvar:: DEBUG_VERBOSE_LEVEL

   When compiled in debug mode (:envvar:SMING_RELEASE undefined) there are four debug levels in increasing level of verbosity:

   * 0: errors
   * 1: warnings
   * 2: information (default)
   * 3: debug

Change it like this::

   make DEBUG_VERBOSE_LEVEL=3


.. envvar:: DEBUG_PRINT_FILENAME_AND_LINE

   Set this to 1 to include the filename and line number in every line of debug output.
   This will require extra space on flash.

.. note::
   If you change these settings and want them applied to Sming, not just your project, then you'll
   need to recompile all components like this::

      make components-clean
      make DEBUG_VERBOSE_LEVEL=3


.. envvar:: STRICT

   * default: undefined (standard warnings, treat as errors)
   * 1: Enable all warnings but do not treat as errors

   By default, Sming builds code with a few warnings disabled:

   - sign-compare: Comparison between signed and unsigned values could produce an incorrect result when the signed value is converted to unsigned.
   - parentheses: Parentheses are omitted in certain contexts.
   - unused-variable: A local or static variable is unused aside from its declaration.
   - unused-but-set-variable: A local variable is assigned to, but otherwise unused.
   - strict-aliasing: Casts which can violate alignment rules.

   These can be indicative of problematic code rather than errors.
   Because new compiler releases can increase the level of checking, this set may also need to change
   but it is kept minimal. See ``Sming/build.mk`` for the actual settings.
   See https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html for further details
   about all compiler settings.
   See also :doc:`/information/develop/clang-tools`.

   Any remaining warnings will be treated as errors and compilation will be halted.

   It is a good idea to check your codebase with ``STRICT=1`` which enables **all** warnings,
   plus a few additional ones such as ``unused-parameter``.

   When enabled, warnings are not treated as errors.


Build caching
~~~~~~~~~~~~~

.. envvar:: ENABLE_CCACHE

   Default: 0 (disabled)

   Set to 1 to run (most) compilation through `ccache <https://ccache.dev/>`__.
   This speeds up re-compilation of code considerably at the expense of disk space
   and slightly extended initial compilation.

   This setting was introduced mainly for CI builds as relatively little changes between runs.


Release builds
~~~~~~~~~~~~~~

.. envvar:: SMING_RELEASE

   By default, this value is undefined to produce a build with debug output.
   To build for release, do this::

      make SMING_RELEASE=1

   This remains in force until you change it back::

      make SMING_RELEASE=


Localisation
~~~~~~~~~~~~

.. envvar:: LOCALE

   Sming can format dates/time values based on a country code identified by this value.
   This is provided as a #define symbol for your application to use.
   See :source:`Sming/Core/SmingLocale.h` for further details.


Networking
~~~~~~~~~~

.. envvar:: DISABLE_NETWORK

   .. note::
   
   0 (Default)
   1 - Remove core networking support

   Applications which do not require networking can set this flag to avoid building
   or linking the core :component:`Network` library.

   This will reduce build times, application size and RAM usage.
   Builds will not succeeded if network code has been inadvertently included.


.. envvar:: DISABLE_WIFI

   .. note::

      EXPERIMENTAL

   0 (Default)
   1 - Exclude WiFi initialisation code

   Keeps the core :component:`Network` library but excludes WiFi code.
   Applications using ethernet can use this to reduce code size. See :sample:`Basic_Ethernet`.


Components
----------

 .. toctree::
   :glob:
   :maxdepth: 1

   Components/*/index
