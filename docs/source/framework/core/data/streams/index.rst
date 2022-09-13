Streams
=======

.. highlight:: c++

Sming provides a set of Stream class which extend :cpp:class:`Stream` methods.

:cpp:class:`IDataSourceStream` is used where read-only access is required.
It introduces the :cpp:func:`IDataSourceStream::readMemoryBlock` method which
performs a regular read without updating the stream position.
This allows optimistic reading and re-sending, but cannot be handled by some stream
types and should be used with care.

:cpp:class:`ReadWriteStream` is used where read/write operation is required.

Printing
--------

The arduino :cpp:class:`Print` class provides the basic output streaming mechanism.
Sming has some enhancements:

C++ streaming operation <<
   Building output is commonly done like this::

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, humidity: ");
      Serial.print(humidity);
      Serial.println("%");

   In Sming, this will produce exactly the same result::

      Serial << "Temperature" << temperature << " °C, humidity: " << humidity << "%" << endl;

   .. note::

      Sming does NOT support the C++ STL streaming classes, such as ``iostream``, etc.


Number Printing
   Examples::

      Serial.print(12, HEX);         // "c"
      Serial.print(12, HEX, 4);      // "000c"
      Serial.print(12, HEX, 4, '.'); // "...c"
      Serial.print(12);              // "12"
      Serial.print(12, DEC, 4);      // "0012"

   Similar extensions are provided for :cpp:class:`String` construction::

      Serial << "0x" << String(12, HEX, 8);       // "0x0000000c"
      Serial << String(12, DEC, 4, '.');          // "..12"


Field-width control
   Supported via String methods::

      Serial << String(12).padLeft(4);          // "  12"
      Serial << String(12).padLeft(4, '0');     // "0012"
      Serial << String(12).padRight(4);         // "12  "
      Serial << String(12).pad(-4, '0');        // "0012"
      Serial << String(12).pad(4);              // "12  "


Strongly-typed enumerations
   Use of ``enum class`` is good practice as it produces strongly-typed and scoped values.
   Most of these are also provided with a standard ``toString(E)`` function overload.

   This allows string equivalents to be printed very easily::

      auto status = HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED;
      auto type = MIME_HTML;
      Serial.print(type); // "text/html"
      // "Status: HTTP Version Not Supported, type: text/html"
      Serial << "Status: " << status << ", type: " << type << endl;
      // Status: 505, type: 0
      Serial << "Status: " << int(status) << ", type: " << int(type) << endl;


API Documentation
-----------------

.. toctree::
   :glob:
   :maxdepth: 1

   *
