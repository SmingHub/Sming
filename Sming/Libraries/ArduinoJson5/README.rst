ArduinoJson Version 5
=====================

.. highlight:: c++

Provided to support existing applications. New projects should use :library:`ArduinoJson6`.

.. attention::

   Issue with JSON keys (applies to version 5 only)

   According to the ArduinoJson docs it should take an internal copy of
   char* strings, but it doesn’t! This can occur using the :c:func:`_F` macro::

      root[_F("offset")] = something;

   This won't work. Instead, use the :c:func:`F` macro::
   
      root[F("offset")] = something;
   
