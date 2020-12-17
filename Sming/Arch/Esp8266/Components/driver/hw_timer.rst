hw_timer: Hardware Timers
=========================

Driver for hardware timers.

Variables
---------

.. envvar:: USE_US_TIMER

   0 (default): Use default /256 prescale for Timer2
   1: Use /16 prescale
   
   The following functions depend on Timer2:
   - NOW() return value, the Timer2 tick count
   - Software timers
   - System time

   Software timers are driven by Timer2, which by default uses a /256 prescale
   providing a resolution of 3.2us and a range of 1' 54".

   Enabling this setting increases the resolution to 200ns but reduces the maximum
   software timer to 7" 9.5s.

API Documentation
-----------------

.. doxygengroup:: hw_timer
   :content-only:
   :members:
