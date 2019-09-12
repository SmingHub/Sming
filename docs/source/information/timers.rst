Timers
======

.. highlight:: c++

Generally, timers in Sming fall into two categories: Callback Timers and Polled Timers

Callback Timers
---------------

A callback timer is operated by programming a time interval, then arming it. When the timer
'fires' a function of your choosing gets called. Timers can be 'one-shot', for timing single
events, or 'auto-reset' repetitive timers. A repetitive timer will ensure that time intervals
between successive callbacks are consistent.

There are two categories of callback timer.

Hardware timers
   The ESP8266 has two main hardware timers. Timer1 is a countdown timer, which triggers
   when a pre-programmed interval has elapsed. The callback handler is executed in an
   :doc:`interrupt <interrupts>` context so is restricted on what it can do.
   The timer is appropriate where accurate and short timing is required.

   The *HardwareTimer* class provides access to Timer1.
   
   Timer2 is an 'up' counter, and is useful for elapse (interval) timing and very short
   time periods where the overhead of an interrupt service routine is too great.

   The *ElapseTimer* class provides access to Timer2.

Software timers
   Software timers are managed as a queue and driven via Timer2 interrupt. All timer callback handlers
   execute in the *task context* so there are no special restrictions on what you can do.
   Because of this, however, these timers are less precise than the hardware timer and the accuracy
   generally depends on how busy the CPU is.

   The *Timer* class is the most flexible way to use software timers, supporting extended time intervals
   and delegate callback functions so you can use it with class methods, capturing lambdas, etc.

   The *SimpleTimer* class uses less RAM, but only supports regular function callbacks.

Polled timers
-------------

Polled timers can be used to measure elapsed time intervals or to check for timeouts within code loops.
Traditionally one might do this as follows::

   const unsigned TIMEOUT_MS = 100;

   unsigned start = millis();
   while(millis() - start < TIMEOUT_MS) {
      // do some stuff
   }
   unsigned elapsed = millis() - start;
   Serial.print("Elapsed time: ");
   Serial.print(elapsed);
   Serial.println("ms");

.. note::

   A common source of bugs when hand-coding such loops can be illustrated by this alternative::

      unsigned timeout = millis() + TIMEOUT_MS;
      while(millis() < timeout) {
         // do some stuff
      }

   At first glance this looks better than the first approach as we don't have a subtraction within the loop.
   However, when millis() exceeds *MAXINT - TIMEOUT_MS* the *timeout* calculation will wrap and the
   loop will never get executed.
   This takes a little under 25 days, but with microseconds it'll happen in less than an hour.
   This may not be the desired behaviour.

It's generally safer and easier to use a Polled Timer::

   OneShotFastMs timer(TIMEOUT_MS);
   while(!timer.expired()) {
      // do some stuff
   }
   auto elapsed = timer.elapsedTime(); // Returns a `NanoTime::Time` object, value with time units
   Serial.print("Elapsed time: ");
   Serial.println(elapsed.toString()); // Includes units in the elapsed time interval
   // Show time rounded to nearest whole seconds
   Serial.println(elapsed.as<NanoTime::Seconds>().toString());

If you prefer to use microseconds, use *OneShotFastUs* instead or specify the units directly::

   OneShotElapseTimer<NanoTime::Microseconds> timer;

Another advantage of polled timers is speed. Every call to millis (or micros) requires a calculation
from clock ticks into milliseconds (or microseconds). It's also a function call.

Polled timers measure time using hardware clock ticks, and query the hardware timer register directly
without any function calls or calculations. This makes them a much better choice for tight timing loops.

Here's the output from the :source:`BenchmarkPolledTimer <tests/HostTests/app/test-clocks.cpp#L231>` module:

.. code-block:: text

   How many loop iterations can we achieve in 100 ms ?
   Using millis(), managed 55984 iterations, average loop time = 1786ns (143 CPU cycles)
   Using micros(), managed 145441 iterations, average loop time = 688ns (55 CPU cycles)
   Using PolledTimer, managed 266653 iterations, average loop time = 375ns (30 CPU cycles)

Timer range
-----------

The maximum interval for a timer varies depending on the clock source and the selected prescaler.
The count may be further restricted by hardware. For example, Timer1 only provides a 23-bit count
which means with a /16 prescaler (the default for HardwareTimer) it overflows after only 1.67 seconds.

It's therefore important to check that timers are being used within their valid range. There are generally
two ways to do this:

Runtime checks
   This means checking function/method return values and acting accordingly.
   This often gets omitted because it can lead to cluttered code, which then leads to undiagnosed
   bugs creeping in which can be very difficult to track down later on.

   With a polled timer, you'd use one of these methods to set the time interval::

      bool reset(const TimeType& timeInterval);
      bool resetTicks(const TimeType& interval);

   They both return true on success.

Static checks
   These checks are performed during code compilation, so if a check fails the code won't compile.
   In regular 'C' code you'd do this using #if statements, but C++ offers a much better way using
   `static_assert <https://en.cppreference.com/w/cpp/language/static_assert>`__.

   To reset a polled timer and incorporate a static check, use this method::   

      template <uint64_t timeInterval> void reset();

   Note that timeInterval cannot be a variable (even if it's *const*) as the compiler must be
   able to determine its value. It must therefore be *constexpr compatible*.

You can use static checking to pre-validate the range for a timer before using it::

   timer.checkTime<10>();
   timer.checkTime<10000>();

This will throw a compile-time error if the timer is not capable of using intervals in the
range 10 - 10000 microseconds (or whichever time unit you've selected). It doesn't add any
code to the application. If the code compiles, then you can be confident that the timer
will function as expected and you don't need to check return values.

Clocks
------

Timers and their capabilities can vary considerably. For example, Timer1 can be configured with
a prescaler of 1, 16 or 256 which affects both the resolution and range of the timer. One might
also consider the CPU cycle counter to have a selectable prescaler of 1 or 2, depending on
whether it's running at 80MHz or 160MHz.

A *Clock* definition is a class template which allows us to query timer properties and perform time
conversions for a specific timer configuration. These definitions can be found in :source:`Sming/Platform/Clocks.h`.

.. note:: A Clock is a purely virtual construct and does not provide any means to configure the hardware,
   although it does provide the *ticks()* method to obtain the current timer value.

Clocks are made more useful by *TimeSource*, a generic class template defined in :source:`Sming/Core/NanoTime.h`.
This provides methods to convert between time values and tick values for a specific time unit.

Let's say we want a microsecond source using Timer2::

   TimeSource<Timer2Clock, NanoTime::Microseconds, uint32_t> t2source;

We can now call methods of *t2source* like this::

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
