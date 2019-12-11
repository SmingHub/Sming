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


.. envvar:: UMM_FUNC_IRAM

   Default: 1 (enabled)

   Custom heap functions are stored in IRAM by default for performance reasons.

   If you need the IRAM (about 1.5K bytes) then disable this option::
   
      make ENABLE_CUSTOM_HEAP=1 UMM_FUNC_IRAM=0
