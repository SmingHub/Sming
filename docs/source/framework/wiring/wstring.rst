Wiring String
=============

Small String Optimisation
-------------------------

The :cpp:class:`String` class is probably the most used class in Arduino and Sming.

Unfortunately it gets the blame for one of the most indidious problems in the
embedded world, `heap fragmentation <https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html>`__.

To alleviate this problem, Sming uses a technique known as *Small String Optimisation*,
which uses the available space inside the String object itself to avoid using the heap for small allocations
of 10 characters or fewer.

This was lifted from the `Arduino Esp8266 core <https://github.com/esp8266/arduino/pull/5690>`.
Superb work - thank you!

Configuration Variables
-----------------------

.. envvar:: STRING_OBJECT_SIZE

   minimum: 12 bytes (default)
   maximum: 128 bytes
   
   Must be an integer multiple of 4 bytes.

   This is an experimental feature which lets you increase the size of a String object to
   reduce heap allocations further. The effect of this will vary depending on your application,
   but you can see some example figures in :pull-request:`1951`.

   Benefits of increasing :envvar:`STRING_OBJECT_SIZE`:
   
   -  Increase code speed
   -  Fewer heap allocations
   
   Drawbacks:
   
   -  Increased static memory usage for global/static String objects or embedded within global/static class instances.
   -  A String can use SSO *or* the heap, but not both together, so in heap mode any additional SSO space will remain unused.

   Allows the size of a String object to be changed to increase the string length available before the heap is used.

   .. note::

      The current implementation uses one byte for a NUL terminator, and another to store the length,
      so the maximum SSO string length is (STRING_OBJECT_SIZE - 2) characters.

      However, the implementation may change so if you need to check the maximum SSO string size
      in your code, please use ``String::SSO_CAPACITY``.

API Documentation
-----------------

.. doxygenclass:: String
