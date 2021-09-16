Sming Esp32 Architecture
==========================

.. highlight:: bash

Support building Sming for the Esp32 architecture.


Build variables
---------------

.. envvar:: IDF_PATH

   This contains the base directory for the ESP-IDF toolchain used to build the framework. This variable is required and must be set accordingly.


.. envvar:: SDK_CUSTOM_CONFIG

   Custom SDK settings for a project can be defined in a separate file
   and setting this value to the location, relative to the project source root directory.

   These will be added to the default SDK settings.

   To make the settings current, you must run ``make sdk-config-clean``.
   This will discard any changes made via ``make sdk-menuconfig``.


Requirements
------------

In order to be able to compile for the ESP32 architecture you should have ESP-IDF v4.3 installed.
Some slight changes are required to enable code to compile correctly for C++,
so a fork has been created here https://github.com/mikee47/esp-idf/tree/sming/release/v4.3
which you may clone.

The Sming installers do all this for you - see :doc:`/getting-started/index`.

You can find further details in the `ESP-IDF documentation <https://docs.espressif.com/projects/esp-idf/en/v4.3/get-started/index.html#installation-step-by-step>`__.

Building
--------

Make sure that the :envvar:`IDF_PATH` is set.
Also make sure that the other ESP-IDF environmental variables are set.

In Linux this can be done using the following command::

  source $SMING_HOME/Tools/export.sh

Build the framework and application as usual, specifying :envvar:`SMING_ARCH` =Esp32. For example::

   cd $SMING_HOME/../samples/Basic_Serial
   make SMING_ARCH=Esp32

This builds the application. Once built the application needs to be flashed on a real Esp32 microcontroller to run.
Flashing can be done using the following command::

  make flash


SDK
---

Sming comes with pre-compiled libraries and configuration files. If needed you can re-configure ESP-IDF using the command below::

  make SMING_ARCH=Esp32 sdk-menuconfig

A re-compilation is required after the change of the configuration. This can be done with the following command::

  make SMING_ARCH=Esp32 Sming-build all

If you want to revert to using the default SDK settings then issue the following command::

  make SMING_ARCH=Esp32 sdk-config-clean

You can also configure per-project custom settings via :envvar:`SDK_CUSTOM_CONFIG`.


Processor variants
------------------

Sming leverages the `ESP IDF HAL <https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-guides/hardware-abstraction.html>`__
to support multiple processor variants.

This is still at an early stage of development however basic applications should build for the following variants:

- esp32 (default)
- esp32s2
- esp32c3
- esp32s3

You can change variants like this:

```
make SMING_ARCH=Esp32 ESP_VARIANT=esp32c3
```

Each variant uses a different build directory, e.g. ``out/Esp32/esp32c3/...`` to avoid conflicts.

See :component-esp32:`esp32` for further details.


Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index
