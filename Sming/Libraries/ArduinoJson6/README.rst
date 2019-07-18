ArduinoJson Version 6
=====================

If you're upgrading from :library:`version 5 <ArduinoJson5>`, some changes will be required to your code.
See the `Version 6 Migration Guide <https://arduinojson.org/v6/doc/upgrade>`__ for details.

Some methods of ``JsonVariant`` have been removed, replacements are:

``asString()`` -> ``as<char*>()`` or ``as<const char*>``. Note that ``as<String>`` produces a
serialized version, so you’ll get “null” instead of an empty/invalid result String.

``asArray()`` -> ``as<JsonArray>()``

``asObject()`` -> ``as<JsonObject>()``


There are also some useful helper functions available in the ``Json`` namespace. See ``Libraries/ArduinoJson6/include/ArduinoJson.h``.

Sming definitions
-----------------

:cpp:class:`JsonObjectStream`
