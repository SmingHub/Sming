Basic Tasks
===========

.. highlight:: c++

This sample demonstrates using the :cpp:class:`Task` class to efficiently perform intensive processing.

Let's suppose we need to continuously sample an analogue input as fast as possible,
do some processing and output the results.

This sample attempts to do some (very rough) fourier analysis on sampled ADC data,
displaying the average amplitudes in a set of frequency 'buckets' (ranges).

The timing jitter using this approach is quite bad, so it attempts to correct but generally this form
of processing is best done with more capable hardware (e.g. ESP32, FPGA).

In this sample application, we just write the results to the debug terminal, but a real application
might stream this to waiting network clients.
