Buffering
=========

In general, writing efficiently to files is best done in chunks, such as by building a line of data in a :cpp:class:`String` and writing it in one go.

Sming offers a simple write-through buffering mechanism which can be used where necessary. The :library:`ArduinoJson` can benefit greatly from this.

.. doxygenclass:: StaticWriteBuffer
   :members:

.. doxygenclass:: HeapWriteBuffer
   :members:

.. doxygenclass:: DynamicWriteBuffer
   :members:
