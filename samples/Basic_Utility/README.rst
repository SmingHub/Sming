Basic Utility
=============

.. highlight:: bash

Demonstrates how to pass parameters to Host applications.

This allows creation of Host-only utility applications.

After building, try this::

   make run HOST_PARAMETERS='command=testWebConstants'

To run the application directly, you can do this::

   out/Host/debug/firmware/app --nonet command=testWebConstants

``--nonet`` is optional. Use ``--help`` to see available emulator options.

See :envvar:`HOST_PARAMETERS` for further details.
