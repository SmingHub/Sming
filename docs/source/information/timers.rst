Timers
------

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
