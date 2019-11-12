CStringArray
============

Introduction
------------

This is a class to manage a double NUL-terminated list of strings, such as "one\0two\0three\0".

It's similar in operation to ``Vector<String>``, but more memory-efficient as all the data is
stored in a single String object. (CStringArray is a subclass of String.)

Background
----------

Each value in the sequence is terminated by a NUL character (\0). It is recommended that
strings are written like this for clarity:

      CStringArray csa = F(
         "one\0"
         "two\0"
         "three\0");

Result: ["one", "two", "three"]

Assignments require a length, so this won't work as expected:

      CStringArray csa =
         "one\0"
         "two\0"
         "three\0";

Result: ["one"]

When assigning sequences, the final NUL separator may be omitted (it will be added automatically):

         "one\0"
         "two\0"
         "three"

Result: ["one", "two", "three"]

Sequences may contain empty values, so this example contains four values:

         "one\0"
         "two\0"
         "three\0"
         "\0"

Result: ["one", "two", "three", ""]

Comparison with Vector<String>
------------------------------

Advantages
   More memory efficient. Uses only a single heap allocation
   Useful for simple lookups, e.g. mapping enumerated values to strings

Disadvantages
   Slower. Items must be iterated using multiple strlen() calls
   Ordering and insertions / deletions not supported

An example of use can be found in :source:`Sming/Core/Network/Http/HttpHeaders.h`.


Use with FlashString
--------------------

You can use a single FlashString containing these values and load them all
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

   You may find :component:`FlashString` Array, Vector or Map more appropriate.

You can see some examples in
:source:`Sming/Core/DateTime.cpp` and
:source:`Sming/Core/Network/WebConstants.cpp`.
