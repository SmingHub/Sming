Hardware timers
===============

The ESP8266 has one of these, :cpp:type:`HardwareTimer`, which can be programmed to trigger
when a pre-programmed interval has elapsed. The callback handler is executed in an
:doc:`interrupt </information/interrupts>` context so is restricted on what it can do.

The timer is appropriate where accurate and short timing is required.

The API for hardware (and :doc:`timer-queue` timers) is identical, implemented using a  
:cpp:class:`CallbackTimer` class template for best performance.


.. doxygengroup:: hardware_timer
   :members:
