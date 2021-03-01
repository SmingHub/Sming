Sming Esp32 Architecture
==========================

Support building Sming for the Esp32 architecture.


Build variables
---------------

.. envvar:: IDF_PATH

   This contains the base directory for the ESP-IDF toolchain used to build the framework. This variable is required and must be set accordingly.

Requirements
------------

In order to be able to compile for the ESP32 architecture you should have ESP-IDF v4.1 installed.
The Sming installers can do this for you - see :doc:`/getting-started/index`.

You can find further details in the `ESP-IDF documentation <https://docs.espressif.com/projects/esp-idf/en/v4.1/get-started/index.html#installation-step-by-step>`__.

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

  make SMING_ARCH=Esp32 sdk-build

If you want to revert to using the default pre-compiled SDK then issue the following command::

  make SMING_ARCH=Esp32 sdk-default

.. note::

   If you have an ESP32-S2 device you'll need to change :envvar:`ESP_VARIANT`::
   
      make ESP_VARIANT=esp32s2

See :component-esp32:`esp32` for further details.

Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index
