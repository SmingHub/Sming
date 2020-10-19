SSL Adapter API
===============

These classes provide the interface between a :cpp:class:`Ssl::Session` and an appropriate adapter.


Error codes
-----------

Error codes are implementation specific, however 0 always indicates success and < 0 for error.

To obtain a description for an error code, use :cpp:func:`Ssl::Connection::getErrorString`.

SSL `Alerts` are reported via error codes. To obtain the alert code call
:cpp:func:`Ssl::Connection::getAlert` which returns an :cpp:enum:`Ssl::Alert` code.
If the error code is not an alert then ``Alert::INVALID`` is returned.

.. doxygenenum:: Ssl::Alert


Classes
-------

.. doxygenclass:: Ssl::Factory
   :members:

.. doxygenclass:: Ssl::Context
   :members:

.. doxygenclass:: Ssl::Connection
   :members:
