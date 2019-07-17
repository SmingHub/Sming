*****************
Arduino Libraries
*****************

Sming comes with a number of ported libraries that cover many areas of
embedded programming but for sure not all. This page provies list of
other libraries that are known to work with Sming or not.

Working libraries
=================

`ClickEncoder <https://github.com/soligen2010/encoder>`__
---------------------------------------------------------

Rotary encoder support. This library is loop-based - requires periodic
call to event collector method, once per 400 ms seems enough and can be
resolved with Timer.

`Keypad <https://github.com/Chris--A/Keypad>`__
-----------------------------------------------

Both polling and interrupt based interfaces work without modifications. Once per 400 ms
polling seems enough and can be resolved with Timer. There is
`I2C version <https://github.com/joeyoung/arduino_keypads>`__ too but
it requires Sming >= 3.2 with I2C subsystem ported from Arduino.

Libraries that are known to not work
====================================

Adapted Arduino libraries
=========================

Some libraries require adaptation for use with Sming, or at least
benefit from couple of small changes. The interface is identical to
original lib or changes are so small that the client code can be reused
on Arduino platform.

`PCF857x <https://github.com/zgoda/Sming_PCF857x>`__
----------------------------------------------------

More features than ``I2CIO.h`` and support for 16-pin PCF8575. Stripped
of unnecessary items, fits only Sming (not interchangeable with
original Arduino code). Library interface not changed so the code is
completely portable.


Included Libraries
==================

These are all the libraries included with Sming:

.. toctree::
   :glob:
   :maxdepth: 1
   :titlesonly:

   /_inc/Sming/Libraries/*/README

