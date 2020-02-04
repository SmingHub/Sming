CStringArray
============

.. highlight:: c++

Introduction
------------

This is a class to manage a double NUL-terminated list of strings, such as ``"one\0two\0three\0"``.

It's similar in operation to ``Vector<String>``, but more memory-efficient as all the data is
stored in a single :cpp:class:`String` object. (CStringArray is a subclass of String.)

You can see some examples in
:source:`Sming/Core/DateTime.cpp` and
:source:`Sming/Core/Network/WebConstants.cpp`.

Background
----------

Each value in the sequence is terminated by a NUL character ``\0``.
For clarity, placing one string per line is suggested::

   // ["one", "two", "three"]
   CStringArray csa = F(
      "one\0"
      "two\0"
      "three\0"
   );

Note use of the :c:func:`F` macro.
Assignments require a length because of the NUL characters, so this won't work as expected::

   // ["one"]
   CStringArray csa =
      "one\0"
      "two\0"
      "three\0";

When assigning sequences, the final NUL separator may be omitted (it will be added automatically)::

   // ["one", "two", "three"]
   CStringArray csa = F(
      "one\0"
      "two\0"
      "three"
   );

Sequences may contain empty values, so this example contains four values::

   // ["one", "two", "three", ""]
   CStringArray csa = F(
      "one\0"
      "two\0"
      "three\0"
      "\0"
   );


Adding strings
--------------

Elements can be added using standard concatenation operators::

   CStringArray arr;
   arr += "string1";
   arr += 12;
   arr += 5.4;
   arr += F("data");

Be mindful that each call may require a heap re-allocation, so consider
estimating or calculating the required space and using :cpp:func:`String::reserve`::

   CStringArray arr;
   arr.reserve(250);
   // now add content


Use with FlashString
--------------------

You can use a single :cpp:type:`FlashString` containing these values and load them all
at the same time into a `CStringArray`::

   DEFINE_FSTR_LOCAL(fstr_list,
      "a\0"
      "b\0"
      "c\0"
      "d\0"
      "e\0"
   );

   CStringArray list(fstr_list);
   for(unsigned i = 0; i < list.count(); ++i) {
      debug_i("list[%u] = '%s'", i, list[i]);
   }

.. note::

   The entire FlashString is loaded into RAM so better suited for occasional lookups
   or if instantiated outside of a loop.

   You may find :cpp:class:`FSTR::Array`, :cpp:class:`FSTR::Vector` or :cpp:class:`FSTR::Map` more appropriate.
   See :component:`FlashString` for details.


Iterator support
----------------

Looping by index is slow because the array must be scanned from the start for each access.
Iterators are simpler to use and much more efficient::

   for(auto s: list) {
      debug_i("'%s'", s);
   }

For more complex operations::

   CStringArray::Iterator pos;
   for(auto it = list.begin(); it != list.end(); ++it) {
      debug_i("list[%u] = '%s' @ %u", it.index(), *it, it.offset());
      // Can use direct comparison with const char* or String
      if(it == "c") {
         pos = it; // Note position
      }
   }
   
   if(pos) {
      debug_i("Item '%s' found at index %u, offset %u", pos.str(), pos.index(), pos.offset());
   } else {
      debug_i("Item not found");
   }


Comparison with Vector<String>
------------------------------

Advantages
   More memory efficient
   Uses only a single heap allocation (assuming content is passed to constructor)
   Useful for simple lookups, e.g. mapping enumerated values to strings

Disadvantages
   Slower. Items must be iterated using multiple strlen() calls
   Ordering and insertions / deletions not supported

An example of use can be found in :source:`Sming/Core/Network/Http/HttpHeaders.h`.


API Documentation
-----------------

.. doxygenclass:: CStringArray
   :members:
