Host UART driver
================

.. highlight:: bash

Introduction
------------

Implements a UART driver to connect via TCP socket, allowing terminal emulation using telnet,
or directly to local host serial device (e.g. /dev/ttyUSB0, COM4, etc.)

If not otherwise reassigned, UART0 output is sent to the console and keyboard input is written to the UART0 receive queue.


Configuration Variables
-----------------------

.. envvar:: ENABLE_HOST_UARTID

   To enable emulation for a UART, set this value to the numbers required.
   You would normally add this to a project's component.mk file.
   
   For example::
   
      ENABLE_HOST_UARTID = 0 1

   If setting it on the command line, remember to use quotes::

      make ENABLE_HOST_UARTID="0 1"

   See :sample:`Basic_Serial` which uses both serial ports like this.


.. envvar:: HOST_UART_PORTBASE

   The base port number to use for telnet. Default is 10000, which corresponds to UART0.
   
   This is passed to the command line ``--uartport`` option.


.. envvar:: HOST_UART_OPTIONS

   By default, this value combines the above options.

   Allows full customisation of UART command-line options for ``make run``.


TCP port emulation
------------------

Set required ports for emulation using the :envvar:`ENABLE_HOST_UARTID`, then execute ``make run``.

.. note::

   As an alternative to ``make run``, you can run the compiled application manually like this::
   
      out/Host/debug/firmware/app --pause --uart=0 --uart=1

Now start a telnet session for each serial port, in separate command windows::

   telnet localhost 10000
   telnet localhost 10001

In the application window, press Enter. This behaviour is enabled by the
``pause`` option, which stops the emulator after initialisation so
telnet can connect to it. Without ``pause`` you’ll lose any serial
output at startup.)

.. note::

   For Windows users, ``putty`` is a good alternative to telnet. It also
   has options for things like carriage-return/linefeed translation
   (“\\n” -> “\\r\\n`”). Run using::
   
      putty telnet://localhost:10000
   
Port numbers are allocated sequentially from 10000. If you want to use
different port numbers, set :envvar:`HOST_UART_PORTBASE`.


Physical device connection
--------------------------

Override :envvar:`HOST_UART_OPTIONS` adding the `--device` option. For example::

   make run HOST_UART_OPTIONS="--uart=0 --device=/dev/ttyUSB0"

The ``--device`` option must follow the ``--uart`` option. Another example::

   make run HOST_UART_OPTIONS="--uart=0 --device=/dev/ttyUSB0 --uart=1 --device=/dev/ttyUSB1"

The port is opened when ``uart_init()`` gets called.

The default baud rate is whatever the application has requested. This can be overridden as follows::

   make run HOST_UART_OPTIONS="--uart=0 --device=/dev/ttyUSB0 --baud=921600"

For Windows, substitute the appropriate device name, e.g. ``COM4`` instead of ``/dev/ttyUSB0``.

.. note::

   If necessary, add ``ENABLE_HOST_UARTID=`` to prevent telnet windows from being created.

Console I/O may be assigned to a different port like this::

   make run HOST_UART_OPTIONS="--uart=1 --device=console"
