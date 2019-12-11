Interrupts
==========

.. highlight:: text

Rules
-----

Normal code runs in a *Task* context, that is to say there are no particular restrictions on what
functions can be called. Sming is a single-threaded framework, so all tasks execute sequentially.

However, there are some rules you need to follow when writing code which runs in an *interrupt context*:

-  Use :c:macro:`IRAM_ATTR` attribute on the interrupt handler
-  Don't call any code which isn't in IRAM, or marked with :c:macro:`__forceinline`.
   Note: ``inline`` by itself is just a compiler 'hint' and there is no guarantee the code will actually be inlined.
-  Keep the code as brief as possible
-  Don't use the heap (malloc, free, new, etc.). Buffers, etc. must be pre-allocated in task context.
-  Put processing code into a separate function and add to the task queue :cpp:func:`SystemClass::queueCallback`,
   or use a :cpp:type:`SimpleTimer` or :cpp:class:`Timer`.
-  Be wary if using :cpp:class:`Delegate` callbacks as the compiler may need to use the heap,
   if parameter lists do not match. For this reason, avoid capturing lambdas and method callbacks.
   If in doubt, stick with regular 'C' function pointers as defined by the :cpp:type:`InterruptCallback` type.

Contact bounce
--------------

If you use a jumper wire to ground the input pin (or even a proper switch) then it will bounce
around as contact is made and then broken. This means you'll get multiple outputs instead
of a clean signal, which will cause havoc with interrupt handlers.

One solution is to use a 'debounce circuit', which can be as simple as this::

                  3v3
                  _|_
                  ___ 100nF
                   |
                   |
   INPUT   >--------------> GPIO PIN
   
Provided pull-ups are enabled on the GPIO, this is sufficient to slow the input.

An alternative solution is to poll the input instead. See :doc:`tasks`.
