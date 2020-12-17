System
======

.. _TaskQueue:

Task Queue
----------

Sming has a *task queue* which allows execution of a function to be deferred until
the system is less busy. This is done by calling :cpp:func:`SystemClass::queueCallback`.

Callbacks are executed as soon as possible, and allow other higher priority tasks
(such as servicing the WiFi stack) to be handled in a timely manner.

A common use for the queue is to initiate processing from an interrupt service routine.

You must not spend too much time in the callback. How much time depends on the
nature of your application, but tasks consuming more than 100ms will probably affect
responsiveness and should be broken into smaller chunks. You might do this by
wrapping such tasks in a class together with some state information. At the end of
the initial callback if there is further work to be done simply make another call
to *queueCallback()*.

The task queue size is fixed, so the call to *queueCallback()* will fail if there is no room.


.. envvar:: TASK_QUEUE_LENGTH

   Maximum number of entries in the task queue (default 10).


.. envvar:: ENABLE_TASK_COUNT

   If problems are suspected with task queuing, it may be getting flooded.
   For this reason you should check the return value from *queueCallback()*.
   
   You can enable this option to keep track of the number of active tasks,
   :cpp:func:`SystemClass::getTaskCount`, and the maximum, :cpp:func:`SystemClass::getMaxTaskCount`.

   By default this is disabled and both methods will return 255.
   This is because interrupts must be disabled to ensure an accurate count,
   which may not be desirable.


API Documentation
-----------------

.. doxygengroup:: system
   :members:
