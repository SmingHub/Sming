Heap
====

Configuration variables
-----------------------

.. envvar:: ENABLE_CUSTOM_HEAP

   If your application is experiencing heap fragmentation then you can try the alternative heap.

   undefined (default)
      OFF, use standard heap

   1
      Use :doc:`UMM Malloc <umm_malloc/README>`.


.. warning::
   Do not enable custom heap allocation and -mforce-l32 compiler flag at the same time.
