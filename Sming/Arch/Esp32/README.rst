Sming Esp32 Architecture
==========================

.. highlight:: bash

Support building Sming for the Esp32 architecture.


Configuration Variables
-----------------------

.. envvar:: IDF_PATH

   Required. The full path to the ESP-IDF framework.

   The standard location for this is ``/opt/esp-idf`` or ``C:\tools\esp-idf``,
   which is a link to the versioned directory such as ``/opt/esp-idf-5.2``.
   You can switch between installed versions by changing the link,
   or by changing IDF_PATH.

.. envvar:: IDF_TOOLS_PATH

   Required. The full path to the Esp32 tools directory, such as ``/opt/esp32``.

.. envvar:: SDK_CUSTOM_CONFIG

   Custom SDK settings for a project can be defined in a separate file
   and setting this value to the location, relative to the project source root directory.

   These will be added to the default SDK settings.

   To make the settings current, you must run ``make sdk-config-clean``.
   This will discard any changes made via ``make sdk-menuconfig``.


Requirements
------------

Sming requires a slightly modified version of the Espressif SDK.
You can find the SDK here https://github.com/mikee47/esp-idf/tree/sming/release/v5.2.
See `idf_versions`_ below for further details.

Using the Sming installation scripts are the recommended way to install these SDK versions.
See :doc:`/getting-started/index`.

You can find further details in the `ESP-IDF documentation <https://docs.espressif.com/projects/esp-idf/en/v4.3/get-started/index.html#installation-step-by-step>`__.

Building
--------

Make sure that the :envvar:`IDF_PATH` is set.
Also make sure that the other ESP-IDF environmental variables are set.

In Linux/MacOS this can be done using the following command::

  source $SMING_HOME/Tools/export.sh

For Windows::

  $SMING_HOME\Tools\export

Build the framework and application as usual, specifying :envvar:`SMING_ARCH=Esp32 <SMING_ARCH>`. For example::

   cd $SMING_HOME/../samples/Basic_Serial
   make SMING_ARCH=Esp32

This builds the application. Once built the application needs to be flashed on a real Esp32 microcontroller to run.
Flashing can be done using the following command::

  make flash


SDK
---

Sming comes with pre-compiled libraries and configuration files. If needed you can re-configure ESP-IDF using the command below::

  make sdk-menuconfig

A re-compilation is required after the change of the configuration thus::

  make Sming-build all

If you want to revert to using the default SDK settings::

  make sdk-config-clean

You can also configure per-project custom settings via :envvar:`SDK_CUSTOM_CONFIG`.


SoC variants
------------

Sming leverages the `ESP IDF HAL <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/hardware-abstraction.html>`__
to support multiple processor variants.

A family of SoCs is supported, currently::

- esp32
- esp32s2
- esp32c3
- esp32s3
- esp32c2

You can change variants like this::

  make SMING_SOC=esp32c3

Each variant uses a different build directory, e.g. ``out/Esp32/esp32c3/...``.

See :component-esp32:`esp32` for further details.


.. _idf_versions:

IDF versions
------------

Sming currently supports IDF versions 5.2. This is installed by default.
Older versions 4.3, 4.4 and 5.0 are no longer officially supported. If you use one of the old versions, please, consider upgrading to 5.2.


A different version can be installed if necessary::

    INSTALL_IDF_VER=4.4 $SMING_HOME/../Tools/install.sh esp32

The installation script creates a soft-link in ``/opt/esp-idf`` pointing to the last version installed.
Use the `IDF_PATH` environment variable or change the soft-link to select which one to use.

After switching versions, run a full clean before re-compiling.
This must include SDK configuration::

  make sdk-config-clean clean components-clean

See `ESP-IDF Versions <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/versions.html>`__
for the IDF release schedule.


Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index
