Events
======

.. highlight:: c++

Introduction
------------

The Sming framework has an `event-driven architecture <https://en.wikipedia.org/wiki/Event-driven_architecture>`__.
In other words, we need to think of our application as performing some *action* in response to an
*event*, input or stimulus.

Software events
---------------

Let's look at the :sample:`Basic_Blink` sample. We define a couple of global variables::

   Timer procTimer;
   bool state = true;

The one to note is *procTimer*, which is a *software timer*.
See :doc:`timers` for how these compare with a *hardware timer*.

The startup entry point for a Sming application is the *init()* function::

   void init()
   {
      pinMode(LED_PIN, OUTPUT);
      procTimer.initializeMs(1000, blink).start();
   }

This sets up a repeating timer, so that every 1000ms the *blink* function gets called::

   void blink()
   {
      digitalWrite(LED_PIN, state);
      state = !state;
   }

Note that what we *don't* do is sit in a loop waiting for something to happen.

Interrupt events
----------------

The :sample:`Basic_Interrupts` sample can be summarised as follows:

-  We have a button connected to GPIO0 as an input
-  Button pressed
   ->  Hardware interrupt on GPIO0
      -> Change output state of LED via GPIO2

There are two ways we can determine when the state of our GPIO0 pin changes:

Polling
   We read the input very quickly and compare the current value with the previous value,
   which we keep a note of. Our *event* occurs when the value changes, or if the input goes HIGH,
   or LOW, etc.

Interrupt
   If we need to catch fast pulses then polling may not work as the CPU might be doing something
   else and we'll miss it. So instead, we'll get the hardware to monitor the input for us and
   generate an *event* when it changes.

Both techniques have advantages and disadvantages, and interrupts are certainly not appropriate in all situations.

Bear in mind that every time an interrupt occurs the CPU has to stop executing your regular
program, save any critical registers then jump to the interrupt service routine to run your
code. All this takes time, so if the input changes very fast and very frequently then it can consume
a lot of CPU and make the system very sluggish (or even crash it).

Callbacks
---------

In Sming, we generally register a *callback* function to be invoked when an event occurs,
such as if a timer expires.

This can be a regular 'C' callback function, which you should use for handling interrupts.

For regular application code, a *Delegate* provides more flexbility and allows you to create
simpler, cleaner code. See `Delegation <https://en.wikipedia.org/wiki/Delegation_(object-oriented_programming)>`__
for a bit of background.

The :sample:`Basic_Delegates` sample shows how to use various types of callback.

One common requirement is to use a class method as a callback, which is easily done using Delegates.

This flexibility comes at a cost, however:

-  Speed. They are slower than their regular C-function counterparts
-  Memory. Some calls may use the heap in the background, the main reason why you should not use
   Delegates in an interrupt context.

See :pull-request:`1734` for some further details about the relative speeds.
