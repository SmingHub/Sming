CPU Usage
=========

.. highlight:: c++

Class to provide a CPU usage indication based on task callback availability.
To produce useful results it requires a couple of seconds at startup to calibrate.

Use like this::

   #include <Services/Profiling/CpuUsage.h>

   // instantiate a single instance of :cpp:class:`Profiling::CpuUsage`
   Profiling::CpuUsage cpuUsage;

   // Will be called when CpuUsage calibration has completed
   void onReady()
   {
      // Continue with application initialisation
   }

   void init()
   {
      // Begin clock calibration
      cpuUsage.begin(onReady);
   }

   
See :sample:`RingTonePlayer` for a more detailed example.

CPU usage is calculated over an update period which begins with a call to :cpp:func:`Profiling::CpuUsage::reset`.
The actual update period must be managed elsewhere, using a callback timer, web request
or other mechanism. It doesn't need to be exact as the actual elapsed time in CPU
cycles is used for the calculation.

After the update period has elapsed, call :cpp:func:`Profiling::CpuUsage::getUtilisation` to obtain a CPU usage figure.

This figure is obtained using the number of task callbacks made within the update period.

loop cycles
   Set up repeating task callback and measure invocations between successive calls

total cycles
   The total number of CPU cycles between calls to :cpp:func:`Profiling::CpuUsage::update`.

used
   total - loop

utilisation
   used / total


.. doxygenclass:: Profiling::CpuUsage
   :members:
