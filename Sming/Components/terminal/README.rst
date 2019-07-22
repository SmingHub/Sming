Terminal
========

.. highlight:: bash

Introduction
------------

This Component provides make targets to support a serial terminal for communicating with devices.

The default serial terminal is `miniterm <https://pyserial.readthedocs.io/en/latest/tools.html#module-serial.tools.miniterm>`__.
If you don't have it installed already, do this::

   pip install pyserial

(You'll need `python <https://www.python.org/>`__, of course.)

Options
-------

.. envvar:: COM_PORT

   Default port for device communications. Default value depends on the development platform being used.

.. envvar:: COM_SPEED_SERIAL

   Baud rate to use. Default is :envvar:`COM_SPEED`.

.. envvar:: COM_OPTS

   Additional options to pass to the terminal.

.. envvar:: TERMINAL

   Command line to use when running ``make terminal``.
   Redefine if you want to use a different terminal application.

.. envvar:: KILL_TERM

   Command line to use to kill the running terminal process
   If the terminal never runs in the background and the warnings annoy you, just clear it.
