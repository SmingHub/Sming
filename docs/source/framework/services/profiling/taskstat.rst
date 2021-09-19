TaskStat
========

.. highlight:: c++

Example of use::

   #include <Services/Profiling/TaskStat.h>

   Profiling::TaskStat taskStat(Serial);
   Timer statTimer;

   void init()
   {
      ...

      // Set up timer to print task information to Serial terminal every 2 seconds
      statTimer.initializeMs<2000>(InterruptCallback([]() { taskStat.update(); }));
      statTimer.start();
   }


.. doxygenclass:: Profiling::TaskStat
   :members:
