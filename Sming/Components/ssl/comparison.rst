Comparison of SSL  implementations
==================================

.. highlight:: C++

Memory usage
------------

**axTLS** uses dynamic (heap) allocations which causes issues with large fragment sizes.
If you run into memory problems, try setting :envvar:`ENABLE_CUSTOM_HEAP`.

**Bear SSL** uses fixed buffers and does not suffer from this limitation.


{ todo }
