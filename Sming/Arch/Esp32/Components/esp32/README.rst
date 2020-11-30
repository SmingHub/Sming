Esp32 Core Component
====================

.. highlight:: bash

Introduction
------------

Contains startup code, crash handling and additional Esp32-specific support code,
including the ESP-IDF SDK.

If you want to tune ESP-IDF to your needs you should run::

    make SMING_ARCH=Esp32 sdk-menuconfig

Followed by::

    make


Configuration variables
-----------------------

These are read-only debug variables:

.. envvar:: SDK_INTERNAL

   **READONLY** When compiled using the current (version 3+) Espressif SDK this value is set to 1.


.. envvar:: SDK_LIBDIR

   **READONLY** Path to the directory containing SDK archive libraries


.. envvar:: SDK_INCDIR

   **READONLY** Path to the directory containing SDK header files


Option variables:

.. envvar:: ESP_VARIANT

   Build for for esp32 or esp32s2 device


The following variables may need to be changed if tools are installed in a different location,
or if multiple versions are installed. By default, the most current version will be used.


.. envvar:: ESP32_COMPILER_PATH

   Location of xtensa compiler toolchain


.. envvar:: ESP32_ULP_PATH

   Location of ULP compiler toolchain
   

.. envvar:: ESP32_OPENOCD_PATH

   Location of ESP32 version of Open OCD JTAG debugging tools.


.. envvar:: ESP32_PYTHON_PATH

   Location of ESP-IDF python.

