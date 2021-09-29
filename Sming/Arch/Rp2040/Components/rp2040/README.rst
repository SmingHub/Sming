RP2040 Core Component
=====================

.. highlight:: bash

Contains startup code, crash handling and additional RP2040-specific support code.

Configuration variables
-----------------------

.. envvar:: PICO_BOARD

    default: pico

    Select development board in use.
    List availlable boards with ``make list-boards``.

    The SDK defines various useful bits of information in a board header file,
    such as the default LED pin, how much flash memory it has, etc.
    Use ``make board-info`` to list these values.

    If using custom hardware, select ``none`` and provide definitions as required.


.. envvar:: ENABLE_BOOTSEL

    default: 1 for debug, 0 for release builds

    This setting is provided to make it easy to re-program RP2040 boards during development.
    When enabled, Sming monitors the BOOTSEL button and restartS in boot mode if pressed.
