Esp32 Core Component
====================

.. highlight:: bash

Contains startup code, crash handling and additional Esp32-specific support code.

If you want to tune ESP-IDF to your needs you should run::

    make sdk-menuconfig

Followed by::

    make sdk-build
   

.. envvar:: SDK_INTERNAL

   **READONLY** When compiled using the current (version 3+) Espressif SDK this value is set to 1.

.. envvar:: SDK_LIBDIR

   **READONLY** Path to the directory containing SDK archive libraries

.. envvar:: SDK_INCDIR

   **READONLY** Path to the directory containing SDK header files

