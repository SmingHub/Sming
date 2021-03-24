Basic Utility
=============

.. highlight:: bash

Demonstrates how to pass parameters to Host applications.

This allows creation of Host-only utility applications.

After building, try this::

   make run HOST_PARAMETERS='command=testWebConstants'

To run the application directly, you can do this::

   out/Host/debug/firmware/app --nonet command=testWebConstants

``--nonet`` is optional.

You may find ``--debug=0`` useful to suppress all but host error messages.
Specify ``1`` to include warnings.

Use ``--help`` to see available emulator options.

See :envvar:`HOST_PARAMETERS` for further details.
