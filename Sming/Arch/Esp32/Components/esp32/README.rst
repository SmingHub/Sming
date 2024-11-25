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



Background
----------

An empty ESP IDF project is built which generates a set of libraries and headers
which the framework can then be built against.

The project is located in ``project/{SMING_SOC}``.

The code for this project is copied from ``sdk/project``.

The default configuration settings are obtained from ``sdk/config`` and written
to ``project/{SMING_SOC}/sdkconfig.defaults``.

When ``sdk-menuconfig`` is run, the ``project/{SMING_SOC}/sdkconfig`` is modified.
This can be reset using ``make sdk-config-clean``.

If custom settings are required for a project then place these in a separate file
and set :envvar:`SDK_CUSTOM_CONFIG` to the location, relative to the project source root directory.
