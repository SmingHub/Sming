Polled timers
=============

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

It's generally safer and easier to use a :cpp:type:`PolledTimer`::

   OneShotFastMs timer(TIMEOUT_MS);
   while(!timer.expired()) {
      // do some stuff
   }
   auto elapsed = timer.elapsedTime(); // Returns a `NanoTime::Time` object, value with time units
   Serial.print("Elapsed time: ");
   Serial.println(elapsed.toString()); // Includes units in the elapsed time interval
   // Show time rounded to nearest whole seconds
   Serial.println(elapsed.as<NanoTime::Seconds>().toString());

If you prefer to use microseconds, use :cpp:type:`OneShotFastUs` instead or specify the units directly::

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


API Documentation
-----------------

.. doxygengroup:: polled_timer
   :members:
