Streams
=======

Sming provides a set of Stream class which extend :cpp:class:`Stream` methods.

:cpp:class:`IDataSourceStream` is used where read-only access is required.
It introduces the :cpp:func:`IDataSourceStream::readMemoryBlock` method which
performs a regular read without updating the stream position.
This allows optimistic reading and re-sending, but cannot be handled by some stream
types and should be used with care.

:cpp:class:`ReadWriteStream` is used where read/write operation is required.


API Documentation
-----------------

.. toctree::
   :glob:
   :maxdepth: 1

   *
