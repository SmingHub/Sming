Hardware timers
===============

The ESP8266 has one of these, :cpp:type:`HardwareTimer`, which can be programmed to trigger
when a pre-programmed interval has elapsed. The callback handler is executed in an
:doc:`interrupt </information/interrupts>` context so is restricted on what it can do.

The timer is appropriate where accurate and short timing is required.

The API for hardware (and :doc:`timer-queue` timers) is identical, implemented using a  
:cpp:class:`CallbackTimer` class template for best performance.


.. note::

   **Applies to Esp8266 architecture only.**

   As with all Callback timers, the :cpp:type:`HardwareTimer` can be one-shot or repeating.

   With the Esp8266 a one-shot timer will repeat after it has been triggered, but only after the timer
   counter wraps around. The Esp8266 only has 23 bits of resolution so with a clock divider of 16
   this will happen after about 1.7s.

   Because the Esp8266 lacks any proper PWM hardware the timer latency is critical.
   Adding any additional code to the callback is therefore left to the programmer.


.. doxygengroup:: hardware_timer
   :members:
