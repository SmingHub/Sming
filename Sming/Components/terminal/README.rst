Terminal
========

Provides make targets to support serial terminal for communicating with devices.

Defines these options:

.. envvar:: COM_PORT

   Default port for device communications. Default value depends on the development platform being used.

.. envvar:: COM_SPEED_SERIAL

   Baud rate to use. Default is :envvar:`COM_SPEED`.

.. envvar:: TERMINAL

   Command line to use when running ``make terminal``

.. envvar:: KILL_TERM

   Command line to use to kill the running terminal process
