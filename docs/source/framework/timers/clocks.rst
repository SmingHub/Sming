Clocks
======

Timers and their capabilities can vary considerably. For example, Timer1 can be configured with
a prescaler of 1, 16 or 256 which affects both the resolution and range of the timer. One might
also consider the CPU cycle counter to have a selectable prescaler of 1 or 2, depending on
whether it's running at 80MHz or 160MHz.

A :cpp:class:`Clock <NanoTime::Clock>` definition is a class template which allows us to query timer properties and perform time
conversions for a specific timer configuration. These definitions can be found in :source:`Sming/Platform/Clocks.h`.

.. note:: A Clock is a purely virtual construct and does not provide any means to configure the hardware,
   although it does provide the :cpp:func:`ticks() <Clock::ticks>` method to obtain the current timer value.

Clocks are made more useful by :cpp:class:`TimeSource <NanoTime::TimeSource>`, a generic class template defined in :source:`Sming/Core/NanoTime.h`.
This provides methods to convert between time values and tick values for a specific time unit.

Let's say we want a microsecond source using Timer2::

   TimeSource<Timer2Clock, NanoTime::Microseconds, uint32_t> t2source;

We can now call methods of ``t2source`` like this::

   // What's the maximum Timer2 value in microseconds?
   Serial.println(t2source.maxClockTime());

   // How many clock ticks per microsecond ?
   Serial.println(t2source.ticksPerUnit()); // 5/1

   // How many clock ticks for 100us ?
   Serial.println(t2source.template timeConst<100>().ticks());

Note that all of these values are computed at compile time. Some runtime conversions::

   Serial.println(t2source.timeToTicks(100));
   Serial.println(t2source.ticksToTime(10000));

The results of conversions are rounded rather than truncated, which provides more accurate
results and reduces timing jitter.

For debugging purposes you can print a description::

   Serial.println(t2source.toString()); // "Timer2Clock/5MHz/32-bit/microseconds"

See :source:`Sming/Core/NanoTime.h` for further details.


System Clock API
----------------

.. doxygengroup:: system_clocks
   :content-only:
