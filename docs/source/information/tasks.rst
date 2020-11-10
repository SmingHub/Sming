Tasks
=====

Introduction
------------

If you need to perform any kind of background processing *task*, then you will need
to consider how to break it up so it executes in small slices to allow other
system functions to continue normally.

You can use callback timers to schedule periodic tasks, but if you simply need to
run the task as soon as possible you should use the :ref:`TaskQueue`.

The BasicTask class
-------------------

This class uses the task queue plus a timer to provide an easy way to write a background task.
All you need to is define a *loop()* function which does the work. The task has three states:

.. seqdiag::
   :caption: Task states
   :align: center
   
   seqdiag task-states {
      activation = none;
      node_width = 80;
      node_height = 60;
      edge_length = 160;
      span_height = 5;
      default_shape = roundedbox;
      default_fontsize = 12; 

      SUSPENDED [label = "suspended"];
      RUNNING [label = "running"];
      SLEEPING [label = "sleeping"];

      SUSPENDED -> RUNNING [label = "resume()"];
      SUSPENDED -> SLEEPING [label = "sleep()"];
      RUNNING -> SLEEPING [label = "sleep()"];
      RUNNING -> SUSPENDED [label = "suspend()"];
      SLEEPING -> RUNNING [label = "resume()"];
      SLEEPING -> RUNNING [label = "timer expired"];
      SLEEPING -> SUSPENDED [label = "suspend()"];
   }

To see this in operation, have a look at the :sample:`Basic_Tasks` sample.


Task Schedulers
---------------

If you want to read further about schedulers, there are various libraries available for
`Arduino <https://github.com/esp8266/arduino>`__.

These are quite simple and generic:

-  `A very simple Arduino task manager <http://bleaklow.com/2010/07/20/a_very_simple_arduino_task_manager.html>`__
-  `TaskScheduler <https://github.com/gadgetstogrow/TaskScheduler>`__

This one is rather more complex:

-  `Task Scheduler <https://github.com/arkhipenko/TaskScheduler>`__

Using a scheduler is a powerful technique which allows the programmer to focus on the task
at hand, rather than how to get it to run.
