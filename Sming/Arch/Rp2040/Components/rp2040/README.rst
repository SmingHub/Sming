RP2040 Core Component
=====================

.. highlight:: bash

Contains startup code, crash handling and additional RP2040-specific support code.

Configuration variables
-----------------------

.. envvar:: PICO_BOARD

    default: pico

    Select development board in use.
    List available boards with ``make list-boards``.

    The SDK defines various useful bits of information in a board header file,
    such as the default LED pin, how much flash memory it has, etc.
    Use ``make board-info`` to list these values.

    If using custom hardware, select ``none`` and provide definitions as required.


.. envvar:: ENABLE_BOOTSEL

    default: 1 for debug, 0 for release builds

    This setting is provided to make it easy to re-program RP2040 boards during development.
    When enabled, Sming monitors the BOOTSEL button and restartS in boot mode if pressed.


.. envvar:: LINK_CYW43_FIRMWARE

    default: 1

    The Pico-W board requires a ~140K (compressed) firmware BLOB which by default is linked into the application image.

    This can be wasteful when using OTA as the firmware must be contained in all application images.

    Setting this value to '0' will omit firmware from the image, and instead load it from a partition called 'cyw43_fw'.
    This partition can be added to the standard map using the 'cyw43_fw' :envvar:`HWCONFIG_OPT` setting::

        make LINK_CYW43_FIRMWARE=0 HWCONFIG_OPT=cyw43_fw

    This is not the default setting since the additional partition must be managed by the end application.

