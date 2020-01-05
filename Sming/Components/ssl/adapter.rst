Adapter API
===========

These classes provide the interface between a :cpp:class:`Ssl::Session` and an appropriate adapter.

 *
 * Returned `int` error codes are 0 for success, or < 0 for error.
 *
 * The error codes themselves are implementation-specific.
 * Use `getErrorString()` to obtain the message.
 * SSL `Alerts` are also reported via error codes. Use `getAlert` 


.. doxygenclass:: Ssl::Factory
.. doxygenclass:: Ssl::Context
.. doxygenclass:: Ssl::Connection
