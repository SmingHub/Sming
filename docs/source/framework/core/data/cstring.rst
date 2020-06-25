CString
=======

.. highlight:: c++

Introduction
------------

Whilst use of `char*` pointers is very common in Sming code, it is generally advisable to avoid pointers in C++ where possible.

The STL provides class templates such as `unique_ptr` which deals with memory alllocation and de-allocation
to avoid issues with memory leaks.

The `CString` class implements this on a `char[]` and adds some additional methods which are similar to the :cpp:class:`String` class.

String vs. CString
------------------

:cpp:class:`String` objects each require a minimum of 24 bytes of RAM, and always contain a length field.
A `CString` is much simpler and contains only a `char*` pointer, so a NULL string is only 4 bytes.

When storing arrays or lists of strings (or objects containing those strings) which change infrequently,
such as fixed configuration data, use a `CString` for memory efficiency.


API Documentation
-----------------

.. doxygenclass:: CString
