Multitasking
============

Pre-emptive Multitasking
------------------------

Modern computers have evolved so that you can write a program and it will (mostly) run
without interfering with anything else that is also running.

With multiple CPUs tasks can actually run at the same time (concurrently), but as there
are always many more threads (tasks) these have to be shared. This is especially true on
older systems which only have a single CPU.

The OS does this using a mechanism called *Pre-emptive Multitasking*. As far as your program
is concerned, it just runs without any apparent interruptions, but in reality it only gets
a little 'slice' of time before the operating system forceably switches to a different program and
lets it run for another 'slice' of time. (Hence the term, *time slicing*.)

With pre-emptive multitasking the operating system has to maintain state for every single
task that is active. This requires additional RAM.

There is also the overhead of switching between tasks. Each task also requires its own
stack space so there is usually more restriction on how it is used.

FreeRTOS is perhaps the most well-known example of a pre-emptive embedded OS.

Co-operative Multitasking
-------------------------

By constrast, *Co-operative Multitasking*, requires applications to 'play fair' and not hog the CPU.
This means that whenever you get called to do some work, you must release control back to the system
in a timely manner.

This technique is well-suited to resource-limited systems such as the Esp8266.

The Esp8266 has only a single CPU, and relatively limited RAM; about 50KBytes is available
for application use. The heap grows from the bottom of RAM, and the stack starts at the top.

Sming uses the ESP8266 Non-OS SDK, which manages much of the low-level system control including
the WiFi stack. It contains the main control loop, which in pseudo-code goes something like this::

   for(;;) {
      ServiceWifi();
      ServiceTimers();
      ServiceTasks();
      FeedWatchdog();
   }

Application code gets called from there in response to a network request, or a timer that you've set up,
and you must deal with the request and return promptly to allow things to continue.

If you don't do this, the system will very likely behave erratically and suffer from dropped WiFi
connections and poor responsiveness. In extreme cases, the system will reset as a self-defence mechanism
via *Watchdog Timer*; If it didn't do this, the device would remain unresponsive until physically reset,
which is generally a bad thing for an embedded device!

.. attention::

   Although there are functions available to manually reset watchdog timers, you should endeavour to avoid
   doing this from application code unless absolutely necessary.
