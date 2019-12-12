Tasks
=====

Introduction
------------

If you need to perform any kind of background processing *task*, then you will need
to consider how to break it up so it executes in small slices to allow other
system functions to continue normally.

You can use callback timers to schedule periodic tasks, but if you simply need to
run the task as soon as possible you should use the :ref:`TaskQueue`.

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
