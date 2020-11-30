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
A detailed installation manual can be found in the `ESP-IDF documentation <https://docs.espressif.com/projects/esp-idf/en/v4.1/get-started/index.html#installation-step-by-step>`__.

Building
--------

Make sure that the IDF_PATH environmental variable is set. Also make sure that the other ESP-IDF environmental variables are set.
For example on Linux this can be done using the following command::

  source $IDF_PATH/export.sh

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


Components
----------

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   Components/sming-arch/index
   Components/*/index
