Program Space
=============

Support for storing and accessing data from Program Space (flash memory).

A string literal (e.g. "string") used in code gets emitted to the *.rodata* segment by the compiler.
That means it gets read into RAM at startup and remains there.

To avoid this, and reclaim the RAM, the data must be stored in a different segment. This is done
using the :c:macro:`PROGMEM` macro.

Such strings are usually defined using the :c:func:`PSTR` macro, which also ensures that any duplicate
strings are merged and thus avoids storing them more than once. This is particularly beneficial
for debugging strings.


Templated code
--------------

.. attention::

   :c:macro:`PROGMEM` may not work when used in templated code.

GCC silently ignores 'section attributes' in templated code, which means const variables will remain
in the default ``.rodata`` section.

Strings defined using :c:func:`PSTR` (and related macros) or :component:`FlashString` definitions
are handled correctly because internally they use special names (``__pstr__`` and ``__fstr__``)
which the linker picks up on.


memcpy_aligned
--------------

Once in flash memory, string data must be read into RAM before it can be used. Accessing the flash
memory directly is awkard. If locations are not strictly accessed as 4-byte words the system will
probably crash; I say 'probably' because sometimes it just behaves weirdly if the RAM address
isn't aligned.

So, the location being accessed, the RAM buffer it's being copied to and the length all have to be
word-aligned, i.e. integer multiples of 4 bytes.
If these conditions are satisfied, then it's safe to use a regular :c:func:`memcpy` call.
However, you are strongly discouraged from doing this.
Instead, use :cpp:func:`memcpy_aligned`, which will check the parameters and raise an assertion in debug mode
if they are incorrect.

FakePgmSpace
------------

Standard string functions such as :c:func:`memcpy_P`, :c:func:`strcpy_P`, etc. are provided to enable
working with P-strings. With the new arduino-provided toolchains these are now part of the standard
library, however Sming has some additions and differences.

:c:func:`F`
   Loads a String object with the given text, which is allocated to flash::
   
      String s = F("test");

   .. note::
   
      The ``F()`` macro differs from the Arduino/Esp8266 implementation in that it instantiates a :cpp:class:`String` object.

      Since the length of the string is known at compile-time, it can be passed to the String
      constructor which avoids an additional call to :c:func:`strlen_P`.


:c:func:`_F`
   Like F() except buffer is allocated on stack. Most useful where nul-terminated data is required::

      m_printf(_F("C-style string\n"));

   This macro is faster than :c:func:`F`, but you need to be careful as the temporary stack
   buffer becomes invalid as soon as the containing block goes out of scope.
   Used as a function parameter, that means the end of the function call.
   
   Examples::
   
      println(_F("Debug started"));
   
      commandOutput->print(_F("Welcome to the Tcp Command executor\r\n"));
   
   
   Bad::
   
      char* s = _F("string")

   An assignment such as this will not work because the temporary will be out of scope after
   the statement, hence s will point to garbage. In this instance ``PSTR_ARRAY(s, "string")`` can be used.


:c:func:`DEFINE_PSTR`
   Declares a PSTR stored in flash. The variable (name) points to flash memory so must be accessed
   using the appropriate xxx_P function.

:c:func:`LOAD_PSTR`
   Loads pre-defined PSTR into buffer on stack::

      // The ``_LOCAL`` macro variants include static allocation
      DEFINE_PSTR_LOCAL(testFlash, "This is a test string\n");
         LOAD_PSTR(test, testFlash)
         m_printf(test);

:c:func:`PSTR_ARRAY`
   Create and load a string into the named stack buffer.
   Unlike :c:func:`_F`, this ensures a loaded string stays in scope::

      String testfunc() {
         //char * test = "This is a string"; <<- BAD
         PSTR_ARRAY(test, "This is a string");
         m_printf(test);
         ...
         return test; // Implicit conversion to String
      }

Both :c:func:`DEFINE_PSTR` and :c:func:`PSTR_ARRAY` load a PSTR into a stack buffer, but using
:c:func:`sizeof` on that buffer will return a larger value than the string itself because it's aligned.
Calling :c:func:`sizeof` on the original flash data will get the right value.
If it's a regular nul-terminated string then :c:func:`strlen_P` will get the length, although it's
time-consuming.

FlashString
-----------

For efficient, fast and flexible use of PROGMEM data see :component:`FlashString`.

API Documentation
-----------------

.. doxygengroup:: pgmspace
   :content-only:
