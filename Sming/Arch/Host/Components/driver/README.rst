Host Drivers
============

Provides low-level peripheral support classes.

* ADC: Analogue functions
* GPIO: General-purpose I/O
* PWM: Common header for PWM support 

UART
----

Implements a UART driver to connect via TCP sockets, allowing terminal emulation using telnet.

.. envvar:: ENABLE_HOST_UARTID

   To enable emulation for a UART, set this value to the numbers required.
   You would normally add this to a project's component.mk file.
   
   For example:
   
   .. code-block:: make
   
      ENABLE_HOST_UARTID = 0 1

   If setting it on the command line, remember to use quotes:
   
   .. code-block:: bash
   
      make ENABLE_HOST_UARTID="0 1"

   See :sample:`Basic_Serial` which uses both serial ports like this.

.. envvar:: HOST_UART_PORTBASE

   The base port number to use for telnet. Default is 10000, which corresponds to UART0.

.. envvar:: HOST_UART_OPTIONS

   The above options are combined to provide command-line options passed to the emulator
   executable. You can change this directly if necessary.
