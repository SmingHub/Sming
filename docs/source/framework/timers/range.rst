Timer range
===========

.. highlight:: c++


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

You can see these checks in action in the :sample:`LiveDebug` sample, which uses the :cpp:type:`HardwareTimer`
with a /16 prescaler. If you change :c:macro:`BLINK_INTERVAL_MS` to 2000 then the code will not compile.

