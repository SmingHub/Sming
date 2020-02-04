CPU Usage
=========

Class to provide a CPU usage indication based on task callback availability.

To use, instantiate a single instance of :cpp:class:`Profiling::CpuUsage`
and call :cpp:func:`Profiling::CpuUsage::begin` from the application's :c:func:`init`
function, passing a callback function to be invoked after calibration has completed.
This function will continue with your application's normal execution.

See :sample:`RingTonePlayer` for an example.

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
