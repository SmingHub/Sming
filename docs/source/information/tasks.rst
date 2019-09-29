Tasks
=====

Introduction
------------

If you need to perform any kind of background processing *task*, then you will need
to consider how to break it up so it executes in small slices to allow other
system functions to continue normally.

You can use callback timers to schedule periodic tasks, but if you simply need to
run the task as soon as possible you should use the task queue.

The task queue
--------------

Sming has a *task queue* which applications can use by calling `System.queueCallback()`:

.. doxygenclass:: SystemClass
   :members: queueCallback

Callbacks are executed as soon as possible, and allow other higher priority tasks
(such as servicing the WiFi stack) to be handled in a timely manner.

.. Note::

   The task queue size is fixed, so the call to *queueCallback* will fail if there
   is no room.

   If you wish to check the actual queue usage set :envvar:`ENABLE_TASK_COUNT`
   and use `System::getMaxTaskCount()` and `System::getTaskCount()` methods.

   If necessary, the task queue size can be changed using :envvar:`TASK_QUEUE_LENGTH`.

You must not spend too much time in the callback. How much time depends on the
nature of your application, but tasks consuming more than 100ms will probably affect
responsiveness and should be broken into smaller chunks. You might do this by
wrapping such tasks in a class together with some state information. At the end of
the initial callback if there is further work to be done simply make another call
to *queueCallback*.

Task Schedulers
---------------

At present Sming doesn't provide any structured (class-based) support for task scheduling,
however there are various scheduling libraries available for
`Arduino <https://github.com/esp8266/arduino>`__.

These are quite simple and generic:

-  `A very simple Arduino task manager <http://bleaklow.com/2010/07/20/a_very_simple_arduino_task_manager.html>`__
-  `TaskScheduler <https://github.com/gadgetstogrow/TaskScheduler>`__

This one is rather more complex:

-  `Task Scheduler <https://github.com/arkhipenko/TaskScheduler>`__

Using a scheduler is a powerful technique which allows the programmer to focus on the task
at hand, rather than how to get it to run.
